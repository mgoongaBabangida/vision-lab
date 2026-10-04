#include "visionlab/opencv/contours_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iomanip>
#include <locale>
#include <sstream>
#include <vector>

namespace visionlab
{

ContoursStage::ContoursStage(bool show_measurements) : show_measurements_(show_measurements)
{
}

std::string_view ContoursStage::name() const noexcept
{
    return show_measurements_ ? "Contour measurements (external)" : "External contours";
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
    if (show_measurements_)
    {
        for (const std::vector<cv::Point>& contour : contours)
        {
            // Geometric area enclosed by the outer polygon, not foreground pixel count.
            const double area = cv::contourArea(contour);
            // true includes the segment from the last point back to the first point.
            const double perimeter = cv::arcLength(contour, true);
            const cv::Rect bounds = cv::boundingRect(contour);
            std::ostringstream label;
            label.imbue(std::locale::classic());
            label << std::fixed << std::setprecision(1) << "A=" << area << " px^2\nP=" << perimeter << " px";
            result.boxes.push_back({static_cast<float>(bounds.x), static_cast<float>(bounds.y), static_cast<float>(bounds.width),
                                    static_cast<float>(bounds.height), label.str()});
        }
    }
}

} // namespace visionlab
