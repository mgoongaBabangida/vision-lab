#include "visionlab/opencv/otsu_threshold_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view OtsuThresholdStage::name() const noexcept
{
    return "Otsu threshold";
}

void OtsuThresholdStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat gray, mask;
    cv::cvtColor(view, gray, cv::COLOR_BGR2GRAY);

    // Otsu chooses one cutoff from this frame's histogram; the supplied 0 is ignored.
    // The return value of threshold() is the automatically chosen cutoff.
    cv::threshold(gray, mask, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
    cv::cvtColor(mask, view, cv::COLOR_GRAY2BGR);
}

} // namespace visionlab
