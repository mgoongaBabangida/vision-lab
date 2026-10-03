#include "visionlab/opencv/grayscale_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view GrayscaleStage::name() const noexcept
{
    return "Grayscale";
}

void GrayscaleStage::process(FrameResult& result)
{
    // Practice 01: the Mat view borrows our pixels; grayscale uses a separate buffer.
    Image& img = result.frame.image;
    cv::Mat mat = {img.height(), img.width(), CV_8UC3, img.data(), img.stride_bytes()};
    cv::Mat gray;
    cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(gray, mat, cv::COLOR_GRAY2BGR);
}

} // namespace visionlab
