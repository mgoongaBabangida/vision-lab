#include "visionlab/opencv/canny_walkthrough.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <utility>
#include <vector>

namespace visionlab
{
namespace
{
constexpr float low_threshold = 100.0f;
constexpr float high_threshold = 200.0f;

struct CannyData
{
    cv::Mat gray, magnitude, angle, thin, classes;
};

enum class Step
{
    Gradient,
    Thin,
    Classify,
    Hysteresis,
    Reference
};

class CannyWalkthroughStage final : public Stage
{
public:
    CannyWalkthroughStage(std::shared_ptr<CannyData> data, Step step) : data_(std::move(data)), step_(step)
    {
    }

    std::string_view name() const noexcept override
    {
        switch (step_)
        {
        case Step::Gradient:
            return "Gradient magnitude";
        case Step::Thin:
            return "Non-maximum suppression (teaching)";
        case Step::Classify:
            return "Thresholds: white strong, orange weak";
        case Step::Hysteresis:
            return "Hysteresis (teaching)";
        case Step::Reference:
            return "OpenCV Canny reference";
        }
        return "Canny walkthrough";
    }

    void process(FrameResult& result) override
    {
        Image& image = result.frame.image;
        cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
        cv::Mat display;
        switch (step_)
        {
        case Step::Gradient:
        {
            // Own the input before replacing frame pixels with a visualization.
            cv::cvtColor(view, data_->gray, cv::COLOR_BGR2GRAY);
            cv::Mat dx, dy;
            cv::Sobel(data_->gray, dx, CV_32F, 1, 0, 3);
            cv::Sobel(data_->gray, dy, CV_32F, 0, 1, 3);
            cv::cartToPolar(dx, dy, data_->magnitude, data_->angle, true);
            cv::convertScaleAbs(data_->magnitude, display);
            break;
        }
        case Step::Thin:
        {
            data_->thin = cv::Mat::zeros(data_->gray.size(), CV_32F);
            for (int y = 1; y < view.rows - 1; ++y)
            {
                for (int x = 1; x < view.cols - 1; ++x)
                {
                    float angle = data_->angle.at<float>(y, x);
                    if (angle >= 180.0f)
                    {
                        angle -= 180.0f;
                    }
                    int ox = 1;
                    int oy = 0;
                    if (angle >= 22.5f && angle < 67.5f)
                    {
                        oy = 1;
                    }
                    else if (angle >= 67.5f && angle < 112.5f)
                    {
                        ox = 0;
                        oy = 1;
                    }
                    else if (angle >= 112.5f && angle < 157.5f)
                    {
                        oy = -1;
                    }
                    const float center = data_->magnitude.at<float>(y, x);
                    // Compare across the edge, along its gradient. Break equal-peak ties on one side.
                    if (center > data_->magnitude.at<float>(y - oy, x - ox) && center >= data_->magnitude.at<float>(y + oy, x + ox))
                    {
                        data_->thin.at<float>(y, x) = center;
                    }
                }
            }
            cv::convertScaleAbs(data_->thin, display);
            break;
        }
        case Step::Classify:
        {
            data_->classes = cv::Mat::zeros(data_->gray.size(), CV_8U);
            view.setTo(cv::Scalar(0, 0, 0));
            for (int y = 0; y < view.rows; ++y)
            {
                for (int x = 0; x < view.cols; ++x)
                {
                    const float strength = data_->thin.at<float>(y, x);
                    const unsigned char label = strength >= high_threshold ? 2 : (strength >= low_threshold ? 1 : 0);
                    data_->classes.at<unsigned char>(y, x) = label;
                    if (label != 0)
                    {
                        view.at<cv::Vec3b>(y, x) = label == 2 ? cv::Vec3b(255, 255, 255) : cv::Vec3b(0, 165, 255);
                    }
                }
            }
            return;
        }
        case Step::Hysteresis:
        {
            display = cv::Mat::zeros(data_->gray.size(), CV_8U);
            std::vector<cv::Point> pending;
            for (int y = 0; y < view.rows; ++y)
            {
                for (int x = 0; x < view.cols; ++x)
                {
                    if (data_->classes.at<unsigned char>(y, x) == 2)
                    {
                        display.at<unsigned char>(y, x) = 255;
                        pending.emplace_back(x, y);
                    }
                }
            }
            // Flood through all eight neighbors, including arbitrarily long chains of weak pixels.
            while (!pending.empty())
            {
                const cv::Point point = pending.back();
                pending.pop_back();
                for (int oy = -1; oy <= 1; ++oy)
                {
                    for (int ox = -1; ox <= 1; ++ox)
                    {
                        const int x = point.x + ox;
                        const int y = point.y + oy;
                        if (x >= 0 && x < view.cols && y >= 0 && y < view.rows && data_->classes.at<unsigned char>(y, x) != 0 &&
                            display.at<unsigned char>(y, x) == 0)
                        {
                            display.at<unsigned char>(y, x) = 255;
                            pending.emplace_back(x, y);
                        }
                    }
                }
            }
            break;
        }
        case Step::Reference:
            // Run on the saved blurred grayscale input, never on a debug visualization.
            cv::Canny(data_->gray, display, low_threshold, high_threshold, 3, true);
            break;
        }
        cv::cvtColor(display, view, cv::COLOR_GRAY2BGR);
    }

private:
    std::shared_ptr<CannyData> data_;
    Step step_;
};
} // namespace

void add_canny_walkthrough(Pipeline& pipeline)
{
    const auto data = std::make_shared<CannyData>();
    for (const Step step : {Step::Gradient, Step::Thin, Step::Classify, Step::Hysteresis, Step::Reference})
    {
        pipeline.add(std::make_unique<CannyWalkthroughStage>(data, step));
    }
}

} // namespace visionlab
