#pragma once

#include "visionlab/frame.hpp"
#include <memory>
#include <string_view>
#include <vector>

namespace visionlab {

class Stage {
public:
    virtual ~Stage() = default;
    virtual std::string_view name() const noexcept = 0;
    virtual void process(FrameResult& result) = 0;
};

// Synchronous, ordered, single-stream pipeline. One instance owns stage state.
// No windowing, event loop, renderer, device, or dependency-specific types here.
class Pipeline {
public:
    void add(std::unique_ptr<Stage> stage);
    FrameResult process(Frame frame);

private:
    std::vector<std::unique_ptr<Stage>> stages_;
};

} // namespace visionlab
