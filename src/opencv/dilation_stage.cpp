#include "visionlab/opencv/dilation_stage.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <stdexcept>

namespace visionlab
{

DilationStage::DilationStage(KernelShape shape, int width, int height) : shape_(shape), width_(width), height_(height)
{
    if (width <= 0 || height <= 0 || width % 2 == 0 || height % 2 == 0)
    {
        throw std::invalid_argument("Dilation kernel dimensions must be positive and odd");
    }
    if (shape != KernelShape::Rectangle && shape != KernelShape::Ellipse && shape != KernelShape::Cross)
    {
        throw std::invalid_argument("Unsupported dilation kernel shape");
    }
}

std::string_view DilationStage::name() const noexcept
{
    return "Dilation";
}

void DilationStage::process(FrameResult& result)
{
    Image& image = result.frame.image;
    cv::Mat view(image.height(), image.width(), CV_8UC3, image.data(), image.stride_bytes());
    const int shape = shape_ == KernelShape::Rectangle ? cv::MORPH_RECT
                      : shape_ == KernelShape::Ellipse ? cv::MORPH_ELLIPSE
                                                       : cv::MORPH_CROSS;
    const cv::Mat kernel = cv::getStructuringElement(shape, cv::Size(width_, height_));
    cv::Mat processed;
    // One iteration with a centered kernel and OpenCV's default morphology border handling.
    // The threshold mask has three identical channels; morphology treats each independently.
    cv::dilate(view, processed, kernel);
    processed.copyTo(view);
}

} // namespace visionlab
