#include "visionlab/opencv/erosion_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace visionlab
{

std::string_view ErosionStage::name() const noexcept
{
    return "Erosion";
}

void ErosionStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::Mat processed;
    // One iteration with a centered kernel and OpenCV's default morphology border handling.
    // The threshold mask has three identical channels; morphology treats each independently.
    cv::erode(view, processed, kernel);
    processed.copyTo(view);
}

} // namespace visionlab
