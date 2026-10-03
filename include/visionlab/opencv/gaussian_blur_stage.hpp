#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Practice 03: the learner implements Gaussian smoothing in process().
class GaussianBlurStage final : public Stage
{
public:
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;
};

} // namespace visionlab
