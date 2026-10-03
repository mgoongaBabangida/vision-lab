#include "visionlab/opencv/adaptive_threshold_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view AdaptiveThresholdStage::name() const noexcept
{
    return "Adaptive threshold";
}

void AdaptiveThresholdStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat gray, mask;
    cv::cvtColor(view, gray, cv::COLOR_BGR2GRAY);

    // Local threshold = Gaussian-weighted neighborhood average minus offset.
    constexpr int block_size = 11; // Odd and greater than one.
    constexpr double offset = 2.0;
    cv::adaptiveThreshold(gray, mask, 255.0, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, block_size, offset);
    cv::cvtColor(mask, view, cv::COLOR_GRAY2BGR);
}

} // namespace visionlab
