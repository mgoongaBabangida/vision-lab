#include "visionlab/opencv/contour_hierarchy_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <string>
#include <vector>

namespace visionlab
{

std::string_view ContourHierarchyStage::name() const noexcept
{
    return "Contour tree: outer / hole / island";
}

void ContourHierarchyStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat mask;
    cv::cvtColor(view, mask, cv::COLOR_BGR2GRAY);
    std::vector<std::vector<cv::Point>> contours;
    // Each entry is [next sibling, previous sibling, first child, parent]. -1 means absent.
    std::vector<cv::Vec4i> hierarchy;
    cv::findContours(mask, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    view.setTo(cv::Scalar(0, 0, 0));
    result.boxes.clear();
    for (std::size_t index = 0; index < contours.size(); ++index)
    {
        int depth = 0;
        for (int ancestor = hierarchy[index][3]; ancestor != -1; ancestor = hierarchy[ancestor][3])
        {
            ++depth;
        }
        // Every step inward alternates foreground and background boundaries.
        const bool hole = depth % 2 != 0;
        const std::string role = hole ? "hole" : (depth == 0 ? "outer" : "island");
        const cv::Scalar color = hole ? cv::Scalar(255, 180, 30) : (depth == 0 ? cv::Scalar(80, 230, 255) : cv::Scalar(220, 80, 220));
        // Draw exactly one contour. The hierarchy is used above, not for recursive drawing.
        cv::drawContours(view, contours, static_cast<int>(index), color, 1, cv::LINE_8);
        const cv::Rect bounds = cv::boundingRect(contours[index]);
        const std::string label =
            "#" + std::to_string(index) + " " + role + " d=" + std::to_string(depth) + "\nparent=" + std::to_string(hierarchy[index][3]);
        result.boxes.push_back({static_cast<float>(bounds.x), static_cast<float>(bounds.y), static_cast<float>(bounds.width),
                                static_cast<float>(bounds.height), label});
    }
    // Contour indices are local to this frame; they are not connected-component or tracking IDs.
}

} // namespace visionlab
