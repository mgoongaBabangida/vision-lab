#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

class SobelSmoothingStage final : public Stage
{
public:
    explicit SobelSmoothingStage(bool smooth);
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;

private:
    bool smooth_;
};

} // namespace visionlab
