#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

class HsvColorStage final : public Stage
{
public:
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;
};

} // namespace visionlab
