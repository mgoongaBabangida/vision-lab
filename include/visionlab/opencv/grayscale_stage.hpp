#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Practice 01: grayscale intensity stored in three equal BGR channels for display.
class GrayscaleStage final : public Stage
{
public:
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;
};

} // namespace visionlab
