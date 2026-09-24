#include "visionlab/pipeline.hpp"
#include <chrono>
#include <stdexcept>
#include <utility>

namespace visionlab
{

void Pipeline::add(std::unique_ptr<Stage> stage)
{
    if (!stage)
    {
        throw std::invalid_argument("Cannot add a null pipeline stage");
    }
    stages_.push_back(std::move(stage));
}

FrameResult Pipeline::process(Frame frame)
{
    FrameResult result{std::move(frame), {}, {}};
    for (const std::unique_ptr<Stage>& stage : stages_)
    {
        const std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        stage->process(result);
        const std::chrono::steady_clock::duration elapsed = std::chrono::steady_clock::now() - start;
        result.timings.push_back({std::string(stage->name()), std::chrono::duration<double, std::milli>(elapsed).count()});
    }
    return result;
}

} // namespace visionlab
