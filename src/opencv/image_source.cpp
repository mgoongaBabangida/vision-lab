#include "visionlab/opencv/image_source.hpp"
#include <opencv2/imgcodecs.hpp>
#include <cstring>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace visionlab
{
namespace
{

class ImageSource final : public FrameSource
{
public:
    explicit ImageSource(const std::filesystem::path& path)
    {
        // Filesystem paths support Unicode names on Windows; decode from memory.
        std::ifstream stream(path, std::ios::binary);
        if (!stream)
        {
            throw std::runtime_error("Cannot open image: " + path.u8string());
        }
        const std::vector<unsigned char> bytes{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        if (bytes.empty())
        {
            throw std::runtime_error("Image file is empty: " + path.u8string());
        }
        const cv::Mat decoded = cv::imdecode(bytes, cv::IMREAD_COLOR);
        if (decoded.empty())
        {
            throw std::runtime_error("Cannot decode image: " + path.u8string());
        }
        Image image(decoded.cols, decoded.rows);
        for (int row = 0; row < decoded.rows; ++row)
        {
            std::memcpy(image.data() + static_cast<std::size_t>(row) * image.stride_bytes(), decoded.ptr(row), image.stride_bytes());
        }
        frame_ = Frame{0, 0.0, std::move(image)};
    }

    std::optional<Frame> next() override
    {
        std::optional<Frame> result = std::move(frame_);
        frame_.reset();
        return result;
    }

private:
    std::optional<Frame> frame_;
};

} // namespace

std::unique_ptr<FrameSource> make_image_source(const std::filesystem::path& path)
{
    return std::make_unique<ImageSource>(path);
}

} // namespace visionlab
