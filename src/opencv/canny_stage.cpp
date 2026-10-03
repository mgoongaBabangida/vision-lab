#include "visionlab/opencv/canny_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view CannyStage::name() const noexcept
{
    return "Canny (exercise)";
}

void CannyStage::process(FrameResult& result)
{
    // Practice 05: see lessons/05-canny.md.
    // TODO 1: Create a temporary BGR8 cv::Mat view of result.frame.image.
    // TODO 2: Convert the view into a separate single-channel gray matrix with COLOR_BGR2GRAY.
    // TODO 3: Fill a separate edges matrix with cv::Canny: low=50, high=150, apertureSize=3, L2gradient=true.
    // TODO 4: Convert edges back into the original BGR8 view with COLOR_GRAY2BGR.
    // Until implemented, this stage leaves the image unchanged.

    Image& img = result.frame.image;
    cv::Mat mat = {img.height(), img.width(), CV_8UC3, img.data(), img.stride_bytes()};
    cv::Mat gray, edges;
    cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
    cv::Canny(gray, edges, 100, 200, 3, true);
    cv::cvtColor(edges, mat, cv::COLOR_GRAY2BGR);

}

} // namespace visionlab
