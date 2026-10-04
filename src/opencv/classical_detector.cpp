#include "visionlab/opencv/classical_detector.hpp"
#include "visionlab/opencv/hsv_color_stage.hpp"
#include "visionlab/opencv/morphology_ex_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <iomanip>
#include <locale>
#include <optional>
#include <sstream>
#include <utility>
#include <vector>

namespace visionlab
{
namespace
{
constexpr double minimum_area = 200.0;
constexpr double minimum_circularity = 0.82;

struct Candidate
{
    BoxOverlay box;
    cv::Point2d centroid;
    bool accepted;
};

struct DetectorData
{
    std::optional<Image> source;
    std::vector<Candidate> candidates;
};

class DetectorMaskStage final : public Stage
{
public:
    explicit DetectorMaskStage(std::shared_ptr<DetectorData> data) : data_(std::move(data))
    {
    }
    std::string_view name() const noexcept override
    {
        return "HSV orange mask";
    }
    void process(FrameResult& result) override
    {
        // Own this frame's source even when snapshots are disabled in headless mode.
        data_->source = result.frame.image;
        data_->candidates.clear();
        HsvColorStage mask;
        mask.process(result);
    }

private:
    std::shared_ptr<DetectorData> data_;
};

class CandidateStage final : public Stage
{
public:
    explicit CandidateStage(std::shared_ptr<DetectorData> data) : data_(std::move(data))
    {
    }
    std::string_view name() const noexcept override
    {
        return "Candidates: area, circularity, decision";
    }
    void process(FrameResult& result) override
    {
        Image& image = result.frame.image;
        cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
        cv::Mat mask;
        cv::cvtColor(view, mask, cv::COLOR_BGR2GRAY);
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Vec4i> hierarchy;
        // CCOMP makes each foreground region a root, including islands inside holes.
        cv::findContours(mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
        data_->candidates.clear();
        result.boxes.clear();
        for (std::size_t index = 0; index < contours.size(); ++index)
        {
            if (hierarchy[index][3] != -1)
            {
                continue; // Hole boundaries are not object candidates.
            }
            const std::vector<cv::Point>& contour = contours[index];
            const double area = cv::contourArea(contour);
            const double perimeter = cv::arcLength(contour, true);
            const double circularity = perimeter > 0.0 ? 4.0 * CV_PI * area / (perimeter * perimeter) : 0.0;
            const cv::Moments moments = cv::moments(contour);
            const bool valid_centroid = std::abs(moments.m00) > 1e-9;
            const cv::Point2d centroid =
                valid_centroid ? cv::Point2d(moments.m10 / moments.m00, moments.m01 / moments.m00) : cv::Point2d(0, 0);
            std::string reason;
            if (!valid_centroid || perimeter <= 0.0)
            {
                reason = "degenerate";
            }
            else if (area < minimum_area)
            {
                reason = "too small";
            }
            else if (hierarchy[index][2] != -1)
            {
                reason = "has hole";
            }
            else if (circularity < minimum_circularity)
            {
                reason = "not round";
            }
            const bool accepted = reason.empty();
            const cv::Rect bounds = cv::boundingRect(contour);
            std::ostringstream label;
            label.imbue(std::locale::classic());
            label << (accepted ? "PASS" : reason) << "\nA=" << std::fixed << std::setprecision(0) << area << " C=" << std::setprecision(2)
                  << circularity;
            const BoxOverlay box{static_cast<float>(bounds.x), static_cast<float>(bounds.y), static_cast<float>(bounds.width),
                                 static_cast<float>(bounds.height), label.str()};
            data_->candidates.push_back({box, centroid, accepted});
            result.boxes.push_back(box);
        }
        // No Mat view is used after replacing the owned image.
        result.frame.image = *data_->source;
    }

private:
    std::shared_ptr<DetectorData> data_;
};

class DetectionStage final : public Stage
{
public:
    explicit DetectionStage(std::shared_ptr<DetectorData> data) : data_(std::move(data))
    {
    }
    std::string_view name() const noexcept override
    {
        return "Detections: solid orange round objects";
    }
    void process(FrameResult& result) override
    {
        result.frame.image = *data_->source;
        result.boxes.clear();
        Image& image = result.frame.image;
        cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
        for (const Candidate& candidate : data_->candidates)
        {
            if (!candidate.accepted)
            {
                continue;
            }
            BoxOverlay box = candidate.box;
            box.label.replace(0, 4, "orange round object");
            result.boxes.push_back(std::move(box));
            cv::drawMarker(view, cv::Point(cvRound(candidate.centroid.x), cvRound(candidate.centroid.y)), cv::Scalar(255, 255, 255),
                           cv::MARKER_CROSS, 11, 1, cv::LINE_8);
        }
    }

private:
    std::shared_ptr<DetectorData> data_;
};
} // namespace

void add_classical_detector(Pipeline& pipeline)
{
    const auto data = std::make_shared<DetectorData>();
    pipeline.add(std::make_unique<DetectorMaskStage>(data));
    pipeline.add(std::make_unique<MorphologyExStage>(MorphologyOperation::Opening));
    pipeline.add(std::make_unique<MorphologyExStage>(MorphologyOperation::Closing));
    pipeline.add(std::make_unique<CandidateStage>(data));
    pipeline.add(std::make_unique<DetectionStage>(data));
}

} // namespace visionlab
