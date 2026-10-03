#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Practice 04: the learner implements the horizontal brightness derivative.
class SobelXStage final : public Stage
{
public:
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;
};

} // namespace visionlab
