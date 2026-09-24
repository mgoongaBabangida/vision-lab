#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace visionlab
{

// Owned, tightly packed BGR8 pixels. Origin: top-left; x right, y down.
// The fixed format makes ownership and debugger inspection explicit initially.
class Image
{
public:
    Image(int width, int height);
    int width() const noexcept
    {
        return width_;
    }
    int height() const noexcept
    {
        return height_;
    }
    std::size_t stride_bytes() const noexcept
    {
        return static_cast<std::size_t>(width_) * 3;
    }
    std::size_t size_bytes() const noexcept
    {
        return pixels_.size();
    }
    std::uint8_t* data() noexcept
    {
        return pixels_.data();
    }
    const std::uint8_t* data() const noexcept
    {
        return pixels_.data();
    }

private:
    int width_;
    int height_;
    std::vector<std::uint8_t> pixels_;
};

} // namespace visionlab
