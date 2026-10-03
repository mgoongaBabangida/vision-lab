#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

enum class KernelShape
{
    Rectangle,
    Ellipse,
    Cross
};

class DilationStage final : public Stage
{
public:
    explicit DilationStage(KernelShape shape = KernelShape::Rectangle, int width = 3, int height = 3);
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;

private:
    KernelShape shape_;
    int width_;
    int height_;
};

} // namespace visionlab
