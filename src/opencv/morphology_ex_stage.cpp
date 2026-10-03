#include "visionlab/opencv/morphology_ex_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>

namespace visionlab
{

MorphologyExStage::MorphologyExStage(MorphologyOperation operation) : operation_(operation)
{
    if (operation != MorphologyOperation::Opening && operation != MorphologyOperation::Closing)
    {
        throw std::invalid_argument("Unsupported morphology operation");
    }
}

std::string_view MorphologyExStage::name() const noexcept
{
    return operation_ == MorphologyOperation::Opening ? "Opening (morphologyEx)" : "Closing (morphologyEx)";
}

void MorphologyExStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    const cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    const int operation = operation_ == MorphologyOperation::Opening ? cv::MORPH_OPEN : cv::MORPH_CLOSE;
    cv::Mat processed;
    // Same centered kernel, one iteration, and default borders as our separate stages.
    cv::morphologyEx(view, processed, operation, kernel);
    processed.copyTo(view);
}

} // namespace visionlab
