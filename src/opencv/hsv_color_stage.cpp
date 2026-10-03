#include "visionlab/opencv/hsv_color_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view HsvColorStage::name() const noexcept
{
    return "HSV orange mask";
}

void HsvColorStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat hsv, mask;
    cv::cvtColor(view, hsv, cv::COLOR_BGR2HSV);

    // Standard 8-bit HSV: H=0..179, S/V=0..255. inRange includes both bounds.
    // Orange hue, with enough saturation and brightness to reject gray and very dark pixels.
    const cv::Scalar lower(5, 100, 80);
    const cv::Scalar upper(25, 255, 255);
    cv::inRange(hsv, lower, upper, mask);

    // HSV is temporary processing data; the viewer receives a BGR8 binary mask.
    cv::cvtColor(mask, view, cv::COLOR_GRAY2BGR);
}

} // namespace visionlab
