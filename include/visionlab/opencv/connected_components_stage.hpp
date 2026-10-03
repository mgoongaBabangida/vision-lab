#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Consumes a BGR binary mask and displays its eight-connected foreground regions.
class ConnectedComponentsStage final : public Stage
{
public:
    explicit ConnectedComponentsStage(int minimum_area = 1);
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;

private:
    int minimum_area_;
    std::string name_;
};

} // namespace visionlab
