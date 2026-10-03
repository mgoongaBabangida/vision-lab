#include "visionlab/opencv/contours_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

namespace visionlab
{

std::string_view ContoursStage::name() const noexcept
{
    return "External contours";
}

void ContoursStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat mask;
    cv::cvtColor(view, mask, cv::COLOR_BGR2GRAY);

    // One vector per boundary; each point is an integer (x, y) image coordinate.
    std::vector<std::vector<cv::Point>> contours;
    // Ignore hole boundaries for this first example. SIMPLE compresses straight runs.
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // Draw a debug image. Analysis uses the mask above, never a preceding colored visualization.
    view.setTo(cv::Scalar(0, 0, 0));
    result.boxes.clear();
    // -1 draws every contour; thickness 1 draws outlines instead of filled regions.
    cv::drawContours(view, contours, -1, cv::Scalar(80, 230, 255), 1, cv::LINE_8);
}

} // namespace visionlab
