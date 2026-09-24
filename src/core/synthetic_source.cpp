#include "visionlab/source.hpp"
#include <cmath>
#include <stdexcept>
#include <utility>

namespace visionlab
{

SyntheticSource::SyntheticSource(std::uint64_t frame_count, int width, int height, double fps)
    : count_(frame_count), width_(width), height_(height), fps_(fps)
{
    if (width <= 0 || height <= 0 || !std::isfinite(fps) || fps <= 0)
    {
        throw std::invalid_argument("Synthetic source needs positive dimensions and finite positive FPS");
    }
}

std::optional<Frame> SyntheticSource::next()
{
    if (next_index_ >= count_)
    {
        return std::nullopt;
    }
    Image image(width_, height_);
    const auto marker_x = static_cast<int>(next_index_ % static_cast<std::uint64_t>(width_));
    for (int y = 0; y < height_; ++y)
    {
        for (int x = 0; x < width_; ++x)
        {
            const auto offset = static_cast<std::size_t>(y) * image.stride_bytes() + static_cast<std::size_t>(x) * 3;
            image.data()[offset] = static_cast<std::uint8_t>(x % 256);
            image.data()[offset + 1] = static_cast<std::uint8_t>(y % 256);
            image.data()[offset + 2] = static_cast<std::uint8_t>(std::abs(x - marker_x) < 8 ? 255 : 32);
        }
    }
    Frame frame{next_index_, static_cast<double>(next_index_) / fps_, std::move(image)};
    ++next_index_;
    return frame;
}

} // namespace visionlab
