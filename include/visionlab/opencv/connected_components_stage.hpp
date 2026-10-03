#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Consumes a BGR binary mask and displays its eight-connected foreground regions.
class ConnectedComponentsStage final : public Stage
{
public:
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;
};

} // namespace visionlab
