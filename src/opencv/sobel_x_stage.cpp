#include "visionlab/opencv/sobel_x_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view SobelXStage::name() const noexcept
{
    return "Sobel magnitude";
}

void SobelXStage::process(FrameResult& result)
{
    // Practice 04: see lessons/04-sobel-x.md.
    // Extended by the learner from the X derivative to combined X/Y gradient magnitude.

    Image& img = result.frame.image;
    cv::Mat mat = {img.height(), img.width(), CV_8UC3, img.data(), img.stride_bytes()};
    cv::Mat sobelX, sobelY, total, display;
    cv::Sobel(mat, sobelX, CV_32F, 1, 0, 3);
    cv::Sobel(mat, sobelY, CV_32F, 0, 1, 3);
    cv::magnitude(sobelX, sobelY, total);
    cv::convertScaleAbs(total, display);
    display.copyTo(mat);
}

} // namespace visionlab
