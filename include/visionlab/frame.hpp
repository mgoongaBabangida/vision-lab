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

struct StageTiming
{
    std::string name;
    double milliseconds;
};

// Image-space rectangles; presentation code chooses their visual appearance.
struct BoxOverlay
{
    float x;
    float y;
    float width;
    float height;
    std::string label;
};

struct StageSnapshot
{
    std::string name;
    Image image;
    std::vector<BoxOverlay> boxes;
    double milliseconds = 0;
};

struct FrameResult
{
    Frame frame;
    std::vector<StageTiming> timings;
    std::vector<BoxOverlay> boxes;
    // Optional source + one owned snapshot after every stage, in execution order.
    std::vector<StageSnapshot> snapshots;
};

} // namespace visionlab
