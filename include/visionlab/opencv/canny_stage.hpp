#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Practice 05: the learner implements Canny edge detection.
class CannyStage final : public Stage
{
public:
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;
};

} // namespace visionlab
