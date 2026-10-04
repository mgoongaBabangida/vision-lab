#include "visionlab/opencv/moments_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>
#include <vector>

namespace visionlab
{

std::string_view MomentsStage::name() const noexcept
{
    return "Centroid: yellow +, box center: cyan circle";
}

void MomentsStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat mask;
    cv::cvtColor(view, mask, cv::COLOR_BGR2GRAY);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    view.setTo(cv::Scalar(0, 0, 0));
    view.setTo(cv::Scalar(45, 45, 45), mask);
    result.boxes.clear();
    for (const std::vector<cv::Point>& contour : contours)
    {
        const cv::Moments moments = cv::moments(contour);
        const cv::Rect bounds = cv::boundingRect(contour);
        // Midpoint of the extreme pixel centers, matching contour coordinates.
        const cv::Point2d box_center(bounds.x + (bounds.width - 1) * 0.5, bounds.y + (bounds.height - 1) * 0.5);
        cv::circle(view, cv::Point(cvRound(box_center.x), cvRound(box_center.y)), 6, cv::Scalar(255, 230, 40), 1, cv::LINE_8);
        std::ostringstream label;
        label.imbue(std::locale::classic());
        label << std::fixed << std::setprecision(1);
        // Point/line contours enclose no area, so their area centroid is undefined.
        if (std::abs(moments.m00) > 1e-9)
        {
            const double cx = moments.m10 / moments.m00;
            const double cy = moments.m01 / moments.m00;
            cv::drawMarker(view, cv::Point(cvRound(cx), cvRound(cy)), cv::Scalar(40, 230, 255), cv::MARKER_CROSS, 9, 1, cv::LINE_8);
            label << "C=(" << cx << "," << cy << ")";
        }
        else
        {
            label << "centroid undefined";
        }
        label << "\nm00=" << moments.m00;
        result.boxes.push_back({static_cast<float>(bounds.x), static_cast<float>(bounds.y), static_cast<float>(bounds.width),
                                static_cast<float>(bounds.height), label.str()});
    }
}

} // namespace visionlab
