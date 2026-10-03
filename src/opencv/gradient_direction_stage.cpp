#include "visionlab/opencv/gradient_direction_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view GradientDirectionStage::name() const noexcept
{
    return "Gradient direction";
}

void GradientDirectionStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat gray, dx, dy, magnitude, angle;
    cv::cvtColor(view, gray, cv::COLOR_BGR2GRAY);
    // Keep signed derivatives: taking absolute values here would lose direction.
    cv::Sobel(gray, dx, CV_32F, 1, 0, 3);
    cv::Sobel(gray, dy, CV_32F, 0, 1, 3);
    cv::cartToPolar(dx, dy, magnitude, angle, true);

    // Fixed gain across frames: magnitude 255 and above appears fully bright.
    // Flat regions stay black. Image y points down, so angles turn clockwise on screen.
    cv::Mat value;
    magnitude.convertTo(value, CV_32F, 1.0 / 255.0);
    cv::min(value, 1.0, value);
    const cv::Mat saturation(gray.size(), CV_32F, cv::Scalar(1.0));
    const cv::Mat channels[] = {angle, saturation, value};
    cv::Mat hsv, bgr;
    // Float HSV uses degrees for H, and 0..1 for S and V.
    cv::merge(channels, 3, hsv);
    cv::cvtColor(hsv, bgr, cv::COLOR_HSV2BGR);
    bgr.convertTo(view, CV_8UC3, 255.0);
}

} // namespace visionlab
