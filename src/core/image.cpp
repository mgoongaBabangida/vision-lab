#include "visionlab/image.hpp"
#include <limits>
#include <stdexcept>

namespace visionlab {

Image::Image(int width, int height) : width_(width), height_(height) {
    if (width <= 0 || height <= 0) {
        throw std::invalid_argument("Image dimensions must be positive");
    }
    const auto w = static_cast<std::size_t>(width);
    const auto h = static_cast<std::size_t>(height);
    const auto max = std::numeric_limits<std::size_t>::max();
    if (w > max / 3 || h > max / (w * 3)) {
        throw std::length_error("Image dimensions overflow the buffer size");
    }
    pixels_.resize(w * h * 3);
}

} // namespace visionlab
