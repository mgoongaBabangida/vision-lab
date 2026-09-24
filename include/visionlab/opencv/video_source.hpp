#pragma once

#include "visionlab/source.hpp"
#include <memory>
#include <string>

namespace visionlab
{

// Factory keeps OpenCV headers and decoder lifetime out of the core interface.
std::unique_ptr<FrameSource> make_video_source(const std::string& path);

} // namespace visionlab
