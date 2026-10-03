#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Practice 02: the learner implements a 3x3 mean filter in process().
class MeanBlurStage final : public Stage
{
public:
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;
};

} // namespace visionlab
