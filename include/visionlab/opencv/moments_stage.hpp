#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

class MomentsStage final : public Stage
{
public:
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;
};

} // namespace visionlab
