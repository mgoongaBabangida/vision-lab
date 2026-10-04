#include "visionlab/opencv/harris_pipeline.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <algorithm>
#include <cmath>
#include <optional>
#include <utility>
#include <vector>

namespace visionlab
{
namespace
{
constexpr double quality_fraction = 0.01;
constexpr int minimum_distance = 8;
constexpr std::size_t maximum_corners = 200;

struct HarrisData
{
    std::optional<Image> source;
    cv::Mat gray, response;
};

enum class HarrisStep
{
    Grayscale,
    Response,
    Corners
};

class HarrisStage final : public Stage
{
public:
    HarrisStage(std::shared_ptr<HarrisData> data, HarrisStep step) : data_(std::move(data)), step_(step)
    {
    }
    std::string_view name() const noexcept override
    {
        switch (step_)
        {
        case HarrisStep::Grayscale:
            return "Grayscale";
        case HarrisStep::Response:
            return "Harris: red positive, blue negative";
        case HarrisStep::Corners:
            return "Selected Harris corners";
        }
        return "Harris";
    }
    void process(FrameResult& result) override
    {
        result.boxes.clear();
        if (step_ == HarrisStep::Corners)
        {
            select_corners(result);
            return;
        }
        Image& image = result.frame.image;
        cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
        if (step_ == HarrisStep::Grayscale)
        {
            data_->source = image;
            cv::cvtColor(view, data_->gray, cv::COLOR_BGR2GRAY);
            cv::cvtColor(data_->gray, view, cv::COLOR_GRAY2BGR);
            return;
        }
        cv::Mat floating;
        data_->gray.convertTo(floating, CV_32F, 1.0 / 255.0);
        // blockSize=3 accumulates local gradient products; ksize=3 is the Sobel aperture.
        cv::cornerHarris(floating, data_->response, 3, 3, 0.04);
        double lowest = 0.0;
        double highest = 0.0;
        cv::minMaxLoc(data_->response, &lowest, &highest);
        const double scale = std::max(std::abs(lowest), std::abs(highest));
        view.setTo(cv::Scalar(0, 0, 0));
        for (int y = 0; y < view.rows; ++y)
        {
            for (int x = 0; x < view.cols; ++x)
            {
                const float response = data_->response.at<float>(y, x);
                const unsigned char strength =
                    scale > 0.0 ? cv::saturate_cast<unsigned char>(255.0 * std::sqrt(std::abs(response) / scale)) : 0;
                view.at<cv::Vec3b>(y, x) = response > 0 ? cv::Vec3b(0, 0, strength) : cv::Vec3b(strength, 0, 0);
            }
        }
    }

private:
    void select_corners(FrameResult& result)
    {
        result.frame.image = *data_->source;
        double maximum = 0.0;
        cv::minMaxLoc(data_->response, nullptr, &maximum);
        if (maximum <= 0.0)
        {
            return;
        }
        cv::Mat local_maximum;
        cv::dilate(data_->response, local_maximum, cv::Mat());
        std::vector<cv::Point> candidates;
        // Exclude a narrow border: reflected neighborhoods are not reliable landmarks.
        for (int y = 3; y < data_->response.rows - 3; ++y)
        {
            for (int x = 3; x < data_->response.cols - 3; ++x)
            {
                const float response = data_->response.at<float>(y, x);
                if (response > maximum * quality_fraction && response == local_maximum.at<float>(y, x))
                {
                    candidates.emplace_back(x, y);
                }
            }
        }
        std::sort(candidates.begin(), candidates.end(),
                  [this](const cv::Point& left, const cv::Point& right)
                  {
                      const float a = data_->response.at<float>(left);
                      const float b = data_->response.at<float>(right);
                      return a != b ? a > b : (left.y != right.y ? left.y < right.y : left.x < right.x);
                  });
        std::vector<cv::Point> selected;
        for (const cv::Point& point : candidates)
        {
            bool near_existing = false;
            for (const cv::Point& existing : selected)
            {
                const double dx = static_cast<double>(point.x) - existing.x;
                const double dy = static_cast<double>(point.y) - existing.y;
                if (dx * dx + dy * dy < minimum_distance * minimum_distance)
                {
                    near_existing = true;
                    break;
                }
            }
            if (!near_existing)
            {
                selected.push_back(point);
                // Small existing overlay boxes mark points; these are not object detections.
                result.boxes.push_back({static_cast<float>(point.x - 3), static_cast<float>(point.y - 3), 6, 6, ""});
                if (selected.size() == maximum_corners)
                {
                    break;
                }
            }
        }
    }
    std::shared_ptr<HarrisData> data_;
    HarrisStep step_;
};
} // namespace

void add_harris_pipeline(Pipeline& pipeline)
{
    const auto data = std::make_shared<HarrisData>();
    for (const HarrisStep step : {HarrisStep::Grayscale, HarrisStep::Response, HarrisStep::Corners})
    {
        pipeline.add(std::make_unique<HarrisStage>(data, step));
    }
}

} // namespace visionlab
