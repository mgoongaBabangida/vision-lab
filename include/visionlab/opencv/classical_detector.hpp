#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Appends an inspectable orange round-object detector, with private per-pipeline state.
void add_classical_detector(Pipeline& pipeline);

} // namespace visionlab
