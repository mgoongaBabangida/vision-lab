#pragma once

#include "visionlab/image.hpp"
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace visionlab
{

struct Frame
{
    std::uint64_t index;
    // Media-relative seconds, not processing wall time. Unknown is nullopt.
    std::optional<double> timestamp_seconds;
    Image image;
};

struct DebugImage
{
    std::string name;
    Image image;
};

struct StageTiming
{
    std::string name;
    double milliseconds;
};

struct FrameResult
{
    Frame frame;
    // Stages can publish snapshots; only a frontend decides how to display them.
    std::vector<DebugImage> debug_images;
    std::vector<StageTiming> timings;
};

} // namespace visionlab
