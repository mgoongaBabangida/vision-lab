#pragma once

#include "visionlab/pipeline.hpp"
#include <optional>

namespace visionlab
{

enum class FeatureMethod
{
    Sift,
    Orb
};

struct FeatureMatchReport
{
    std::size_t source_keypoints = 0;
    std::size_t target_keypoints = 0;
    std::size_t ratio_matches = 0;
    std::size_t inliers = 0;
    // Independent check against the known synthetic transform, in working-image pixels.
    std::optional<double> median_known_transform_error;
};

void add_feature_matching(Pipeline& pipeline, FeatureMethod method, std::shared_ptr<FeatureMatchReport> report = nullptr);

} // namespace visionlab
