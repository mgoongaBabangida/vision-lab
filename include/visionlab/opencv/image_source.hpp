#pragma once

#include "visionlab/source.hpp"
#include <filesystem>
#include <memory>

namespace visionlab
{

std::unique_ptr<FrameSource> make_image_source(const std::filesystem::path& path);

} // namespace visionlab
