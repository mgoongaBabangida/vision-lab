#pragma once

#include "visionlab/pipeline.hpp"

namespace visionlab
{

// Teaching contract: input boxes are detections in current-image coordinates.
// Output boxes keep their geometry and receive ID labels. No appearance or motion model.
class NearestNeighborStage final : public Stage
{
public:
    explicit NearestNeighborStage(double maximum_distance = 50.0);
    std::string_view name() const noexcept override;
    void process(FrameResult& result) override;

private:
    struct Track
    {
        std::uint64_t id;
        double x;
        double y;
    };
    double maximum_distance_;
    std::vector<Track> previous_;
    std::uint64_t next_id_ = 1;
    std::optional<std::uint64_t> previous_frame_;
    int width_ = 0;
    int height_ = 0;
};

} // namespace visionlab
