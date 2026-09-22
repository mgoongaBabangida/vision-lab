#include "visionlab/opencv/video_source.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <stdexcept>
#include <utility>

namespace visionlab {
namespace {

class VideoSource final : public FrameSource {
public:
    explicit VideoSource(const std::string& path) {
        if (!std::filesystem::is_regular_file(path) || !capture_.open(path)) {
            throw std::runtime_error("Cannot open video file: " + path);
        }
        fps_ = capture_.get(cv::CAP_PROP_FPS);
    }

    std::optional<Frame> next() override {
        cv::Mat decoded;
        if (!capture_.read(decoded) || decoded.empty()) {
            // VideoCapture cannot reliably distinguish EOF from a decode failure.
            if (index_ == 0) {
                throw std::runtime_error("The video contained no decodable frames");
            }
            return std::nullopt;
        }
        if (decoded.depth() != CV_8U) {
            throw std::runtime_error("Only 8-bit decoded video is currently supported");
        }
        cv::Mat bgr;
        if (decoded.channels() == 3) {
            bgr = decoded;
        } else if (decoded.channels() == 1) {
            cv::cvtColor(decoded, bgr, cv::COLOR_GRAY2BGR);
        } else if (decoded.channels() == 4) {
            cv::cvtColor(decoded, bgr, cv::COLOR_BGRA2BGR);
        } else {
            throw std::runtime_error("Unsupported decoded channel count");
        }
        Image image(bgr.cols, bgr.rows);
        // Copy row by row: cv::Mat may be strided and its storage is reused.
        for (int row = 0; row < bgr.rows; ++row) {
            std::memcpy(image.data() + static_cast<std::size_t>(row) * image.stride_bytes(),
                        bgr.ptr(row), image.stride_bytes());
        }
        std::optional<double> timestamp;
        // A nominal CFR estimate, not a presentation timestamp for VFR footage.
        if (std::isfinite(fps_) && fps_ > 0) {
            timestamp = static_cast<double>(index_) / fps_;
        }
        return Frame{index_++, timestamp, std::move(image)};
    }

private:
    cv::VideoCapture capture_;
    double fps_ = 0;
    std::uint64_t index_ = 0;
};

} // namespace

std::unique_ptr<FrameSource> make_video_source(const std::string& path) {
    return std::make_unique<VideoSource>(path);
}

} // namespace visionlab
