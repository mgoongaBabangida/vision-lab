#include "visionlab/opencv/video_source.hpp"
#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdexcept>

namespace
{
void require(bool value, const char* message)
{
    if (!value)
    {
        throw std::runtime_error(message);
    }
}
} // namespace

int main()
{
    // Fixture lives in the CTest build directory and never enters the repository.
    const std::filesystem::path fixture = std::filesystem::current_path() / "visionlab-test.avi";
    try
    {
        cv::VideoWriter writer(fixture.string(), cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 25.0, cv::Size(32, 24));
        require(writer.isOpened(), "MJPEG writer unavailable; cannot generate video test fixture");
        writer.write(cv::Mat(24, 32, CV_8UC3, cv::Scalar(0, 0, 255)));
        writer.write(cv::Mat(24, 32, CV_8UC3, cv::Scalar(255, 0, 0)));
        writer.release();
        std::unique_ptr<visionlab::FrameSource> source = visionlab::make_video_source(fixture.string());
        const std::optional<visionlab::Frame> first = source->next();
        const std::optional<visionlab::Frame> second = source->next();
        require(first && second && !source->next(), "Video frame count and EOF");
        require(first->index == 0 && second->index == 1, "Video indices");
        require(first->image.width() == 32 && first->image.height() == 24, "Video dimensions");
        require(first->image.data()[2] > 240 && first->image.data()[0] < 15 && second->image.data()[0] > 240,
                "BGR order and independent decoded storage");
        require(second->timestamp_seconds && std::abs(*second->timestamp_seconds - 0.04) < 1e-6, "Nominal media time");
        source.reset();
        std::filesystem::remove(fixture);
        bool rejected = false;
        try
        {
            visionlab::make_video_source(fixture.string());
        }
        catch (const std::runtime_error&)
        {
            rejected = true;
        }
        require(rejected, "Missing video must fail clearly");
        std::cout << "OpenCV video contracts passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        std::error_code ignored;
        std::filesystem::remove(fixture, ignored);
        return 1;
    }
}
