#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Consumes a BGR binary mask and displays only its outermost contours.
class ContoursStage final : public Stage
{
public:
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;
};

} // namespace visionlab
