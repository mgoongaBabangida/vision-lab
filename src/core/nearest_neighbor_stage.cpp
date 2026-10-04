#include "visionlab/nearest_neighbor_stage.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace visionlab
{

NearestNeighborStage::NearestNeighborStage(double maximum_distance) : maximum_distance_(maximum_distance)
{
    if (!std::isfinite(maximum_distance) || maximum_distance <= 0.0)
    {
        throw std::invalid_argument("Matching distance must be finite and positive");
    }
}

std::string_view NearestNeighborStage::name() const noexcept
{
    return "Nearest-neighbor IDs (box centers)";
}

void NearestNeighborStage::process(FrameResult& result)
{
    struct Pair
    {
        double distance;
        std::size_t track;
        std::size_t detection;
    };
    // A fresh pipeline handles normal viewer restart/source changes. Also guard direct callers.
    if (previous_frame_ && (*previous_frame_ == std::numeric_limits<std::uint64_t>::max() || result.frame.index != *previous_frame_ + 1 ||
                            width_ != result.frame.image.width() || height_ != result.frame.image.height()))
    {
        previous_.clear();
        next_id_ = 1;
    }
    std::vector<Track> current;
    for (const BoxOverlay& box : result.boxes)
    {
        current.push_back({0, box.x + box.width * 0.5, box.y + box.height * 0.5});
    }
    std::vector<Pair> pairs;
    for (std::size_t track = 0; track < previous_.size(); ++track)
    {
        for (std::size_t detection = 0; detection < current.size(); ++detection)
        {
            const double distance = std::hypot(previous_[track].x - current[detection].x, previous_[track].y - current[detection].y);
            if (distance <= maximum_distance_)
            {
                pairs.push_back({distance, track, detection});
            }
        }
    }
    // Greedy shortest-first assignment, with deterministic ties. This is not globally optimal assignment.
    std::sort(pairs.begin(), pairs.end(),
              [](const Pair& a, const Pair& b)
              {
                  if (a.distance != b.distance)
                  {
                      return a.distance < b.distance;
                  }
                  return a.track != b.track ? a.track < b.track : a.detection < b.detection;
              });
    std::vector<bool> used(previous_.size(), false);
    for (const Pair& pair : pairs)
    {
        if (!used[pair.track] && current[pair.detection].id == 0)
        {
            current[pair.detection].id = previous_[pair.track].id;
            used[pair.track] = true;
        }
    }
    for (std::size_t index = 0; index < current.size(); ++index)
    {
        const bool is_new = current[index].id == 0;
        if (is_new)
        {
            current[index].id = next_id_++;
        }
        result.boxes[index].label = "ID " + std::to_string(current[index].id) + (is_new ? " (new)" : " (matched)");
    }
    // Only this frame's detections survive: no prediction and no grace period for missed detections.
    previous_ = std::move(current);
    previous_frame_ = result.frame.index;
    width_ = result.frame.image.width();
    height_ = result.frame.image.height();
}

} // namespace visionlab
