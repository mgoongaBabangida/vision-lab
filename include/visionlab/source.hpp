#pragma once

#include "visionlab/frame.hpp"
#include <optional>

namespace visionlab {

class FrameSource {
public:
    virtual ~FrameSource() = default;
    // nullopt means end of input. Configuration/processing errors throw.
    virtual std::optional<Frame> next() = 0;
};

class SyntheticSource final : public FrameSource {
public:
    explicit SyntheticSource(std::uint64_t frame_count = 300,
                             int width = 320, int height = 240, double fps = 30.0);
    std::optional<Frame> next() override;

private:
    std::uint64_t count_;
    int width_;
    int height_;
    double fps_;
    std::uint64_t next_index_ = 0;
};

} // namespace visionlab
