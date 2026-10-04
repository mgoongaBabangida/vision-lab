#include "visionlab/opencv/rotated_rectangle_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <iomanip>
#include <locale>
#include <sstream>
#include <vector>

namespace visionlab
{

std::string_view RotatedRectangleStage::name() const noexcept
{
    return "Boxes: green axis-aligned, cyan rotated";
}

void RotatedRectangleStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    cv::Mat mask;
    cv::cvtColor(view, mask, cv::COLOR_BGR2GRAY);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    // A dim filled mask makes both fitted rectangles easy to see.
    view.setTo(cv::Scalar(0, 0, 0));
    view.setTo(cv::Scalar(45, 45, 45), mask);
    result.boxes.clear();
    for (const std::vector<cv::Point>& contour : contours)
    {
        const cv::Rect aligned = cv::boundingRect(contour);
        const cv::RotatedRect rotated = cv::minAreaRect(contour);
        // Float center, size and angle describe the box; points() gives its four corners.
        cv::Point2f corners[4];
        rotated.points(corners);
        for (int index = 0; index < 4; ++index)
        {
            const cv::Point2f first = corners[index];
            const cv::Point2f second = corners[(index + 1) % 4];
            cv::line(view, cv::Point(cvRound(first.x), cvRound(first.y)), cv::Point(cvRound(second.x), cvRound(second.y)),
                     cv::Scalar(255, 230, 40), 1, cv::LINE_8);
        }
        std::ostringstream label;
        label.imbue(std::locale::classic());
        label << std::fixed << std::setprecision(1) << "rot " << rotated.size.width << "x" << rotated.size.height
              << "\nangle=" << rotated.angle << " deg";
        result.boxes.push_back({static_cast<float>(aligned.x), static_cast<float>(aligned.y), static_cast<float>(aligned.width),
                                static_cast<float>(aligned.height), label.str()});
    }
}

} // namespace visionlab
