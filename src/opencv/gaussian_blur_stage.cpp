#include "visionlab/opencv/gaussian_blur_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view GaussianBlurStage::name() const noexcept
{
    return "Gaussian blur (exercise)";
}

void GaussianBlurStage::process(FrameResult& result)
{
    // Practice 03: original Lesson 2, section 2. See lessons/03-gaussian-blur.md.
    // TODO 1: Create a temporary BGR8 cv::Mat view of result.frame.image.
    // TODO 2: Fill a separate cv::Mat using cv::GaussianBlur, a 5x5 kernel, and sigmaX = 1.5.
    // TODO 3: Copy the resulting pixels back into the original view using copyTo.
    // Until you implement these steps, this stage leaves the image unchanged.

    Image& img = result.frame.image;
    cv::Mat mat = {img.height(), img.width(), CV_8UC3, img.data(), img.stride_bytes()};
    cv::Mat blurred;
    cv::GaussianBlur(mat, blurred, cv::Size(5, 5), 3.5);
    blurred.copyTo(mat);
    (void)result;
}

} // namespace visionlab
