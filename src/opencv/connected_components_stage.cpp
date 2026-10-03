#include "visionlab/opencv/connected_components_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <array>
#include <string>

namespace visionlab
{

std::string_view ConnectedComponentsStage::name() const noexcept
{
    return "Connected components (8-connected)";
}

void ConnectedComponentsStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat mask, labels, stats, centroids;
    cv::cvtColor(view, mask, cv::COLOR_BGR2GRAY);
    // Zero is background. Each nonzero pixel belongs to one foreground component.
    // Labels are CV_32S, stats CV_32S, and centroids CV_64F. Count includes background.
    const int count = cv::connectedComponentsWithStats(mask, labels, stats, centroids, 8, CV_32S);
    const std::array<cv::Vec3b, 6> palette{cv::Vec3b(180, 80, 30),  cv::Vec3b(70, 60, 190),  cv::Vec3b(40, 160, 100),
                                           cv::Vec3b(170, 60, 160), cv::Vec3b(30, 150, 190), cv::Vec3b(170, 150, 40)};
    for (int y = 0; y < view.rows; ++y)
    {
        for (int x = 0; x < view.cols; ++x)
        {
            const int label = labels.at<int>(y, x);
            view.at<cv::Vec3b>(y, x) = label == 0 ? cv::Vec3b(0, 0, 0) : palette[(label - 1) % palette.size()];
        }
    }
    result.boxes.clear();
    for (int label = 1; label < count; ++label)
    {
        const int area = stats.at<int>(label, cv::CC_STAT_AREA);
        result.boxes.push_back(
            {static_cast<float>(stats.at<int>(label, cv::CC_STAT_LEFT)), static_cast<float>(stats.at<int>(label, cv::CC_STAT_TOP)),
             static_cast<float>(stats.at<int>(label, cv::CC_STAT_WIDTH)), static_cast<float>(stats.at<int>(label, cv::CC_STAT_HEIGHT)),
             "#" + std::to_string(label) + " area=" + std::to_string(area)});
    }
    // IDs/colors describe this frame only. They are not persistent tracking IDs.
}

} // namespace visionlab
