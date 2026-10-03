#include "visionlab/opencv/mean_blur_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view MeanBlurStage::name() const noexcept
{
    return "Mean blur (exercise)";
}

void MeanBlurStage::process(FrameResult& result)
{
    // Practice 02: original Lesson 2, section 1. See lessons/02-mean-blur.md.
    // TODO 1: Create a temporary BGR8 cv::Mat view of result.frame.image, as in Practice 01.
    // TODO 2: Use cv::blur with a 3x3 kernel to fill a separate cv::Mat named blurred.
    // TODO 3: Copy the resulting pixels back into the original view using cv::Mat::copyTo.
    // Until you implement these steps, the stage deliberately leaves the image unchanged.

    Image& img = result.frame.image;
    cv::Mat mat = {img.height(), img.width(), CV_8UC3, img.data(), img.stride_bytes()};
    cv::Mat blurred;
    cv::blur(mat, blurred, cv::Size(3, 3));
    blurred.copyTo(mat);
    (void)result;
}

} // namespace visionlab
