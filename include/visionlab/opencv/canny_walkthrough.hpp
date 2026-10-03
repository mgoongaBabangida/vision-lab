#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Appends teaching stages with private, per-pipeline working data.
// Input should already be grayscale and blurred; all displayed outputs are BGR8.
void add_canny_walkthrough(Pipeline& pipeline);

} // namespace visionlab
