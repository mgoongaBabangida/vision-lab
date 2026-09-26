#include "visionlab/opencv/video_source.hpp"
#include "visionlab/opencv/image_source.hpp"
#include "viewer_session.hpp"
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
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
    const std::filesystem::path still_fixture = std::filesystem::current_path() / "visionlab-test.png";
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
        {
            const visionlab::app::PipelineCatalog video_pipelines;
            visionlab::app::ViewerSession video_session(video_pipelines, 10);
            require(video_session.select_source(visionlab::app::SourceCatalog::from_path(fixture)), "Open video session");
            require(video_session.next_frame() && !video_session.next_frame() && video_session.ended(), "Detect actual video EOF");
            require(video_session.previous_frame() && !video_session.ended() && video_session.snapshot()->image.data()[2] > 240,
                    "Previous frame restores the red frame after EOF");
            require(video_session.next_frame() && video_session.ended() && video_session.snapshot()->image.data()[0] > 240,
                    "Forward history restores the blue frame without another read at EOF");
            require(video_session.previous_frame() && video_session.select_pipeline("pass-through") && video_session.next_frame() &&
                        video_session.result()->frame.index == 1 && video_session.snapshot()->image.data()[0] > 240,
                    "Pipeline change on an earlier video frame resumes from its successor");
        }
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
        require(cv::imwrite(still_fixture.string(), cv::Mat(13, 17, CV_8UC3, cv::Scalar(11, 22, 33))), "Write image fixture");
        std::unique_ptr<visionlab::FrameSource> still = visionlab::make_image_source(still_fixture);
        const std::optional<visionlab::Frame> image = still->next();
        require(image && image->image.width() == 17 && image->image.height() == 13 && image->image.data()[0] == 11 &&
                    image->image.data()[2] == 33 && !still->next() && !still->next(),
                "Image decode, BGR layout, and one-frame EOF");
        visionlab::app::PipelineCatalog pipelines;
        visionlab::app::ViewerSession session(pipelines, 10);
        require(session.select_source(visionlab::app::SourceCatalog::from_path(still_fixture)), "Switch session to image");
        require(session.ended() && !session.playing() && session.snapshot()->image.width() == 17,
                "Still images stay visible with playback disabled");
        require(session.restart() && session.snapshot()->image.data()[1] == 22, "Restart image source");
        require(!session.can_previous_frame() && !session.previous_frame(), "Still images have no previous frame");
        std::filesystem::remove(still_fixture);
        std::cout << "OpenCV video contracts passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        std::error_code ignored;
        std::filesystem::remove(fixture, ignored);
        std::filesystem::remove(still_fixture, ignored);
        return 1;
    }
}
