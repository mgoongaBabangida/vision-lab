#include "visionlab/opencv/sobel_smoothing_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

SobelSmoothingStage::SobelSmoothingStage(bool smooth) : smooth_(smooth)
{
}

std::string_view SobelSmoothingStage::name() const noexcept
{
    return smooth_ ? "Sobel X - vertical smoothing" : "X difference - no smoothing";
}

void SobelSmoothingStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat gray, dx, display;
    cv::cvtColor(view, gray, cv::COLOR_BGR2GRAY);
    // ksize=1: horizontal [-1, 0, 1], without perpendicular smoothing.
    // ksize=3: the same difference, averaged vertically with [1, 2, 1] / 4.
    // Dividing the 3x3 response by four matches the gain of the unsmoothed difference.
    cv::Sobel(gray, dx, CV_32F, 1, 0, smooth_ ? 3 : 1, smooth_ ? 0.25 : 1.0);
    cv::convertScaleAbs(dx, display);
    cv::cvtColor(display, view, cv::COLOR_GRAY2BGR);
}

} // namespace visionlab
