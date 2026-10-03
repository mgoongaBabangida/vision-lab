#include "visionlab/opencv/binary_threshold_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view BinaryThresholdStage::name() const noexcept
{
    return "Binary threshold";
}

void BinaryThresholdStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat gray, mask;
    cv::cvtColor(view, gray, cv::COLOR_BGR2GRAY);

    // Pixels strictly above the cutoff become white; the rest become black.
    constexpr double threshold = 127.0;
    cv::threshold(gray, mask, threshold, 255.0, cv::THRESH_BINARY);

    // The viewer's image storage remains BGR8, even for a binary mask.
    cv::cvtColor(mask, view, cv::COLOR_GRAY2BGR);
}

} // namespace visionlab
