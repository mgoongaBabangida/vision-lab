#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

enum class MorphologyOperation
{
    Opening,
    Closing
};

class MorphologyExStage final : public Stage
{
public:
    explicit MorphologyExStage(MorphologyOperation operation);
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;

private:
    MorphologyOperation operation_;
};

} // namespace visionlab
