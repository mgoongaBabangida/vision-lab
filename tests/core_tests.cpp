#include "visionlab/nearest_neighbor_stage.hpp"
#include "visionlab/pipeline.hpp"
#include "visionlab/source.hpp"
#include <cmath>
#include <cstring>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <utility>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

template <typename Function> void rejects(Function function)
{
    bool thrown = false;
    try
    {
        function();
    }
    catch (const std::invalid_argument&)
    {
        thrown = true;
    }
    require(thrown, "Expected invalid_argument");
}

class AddStage final : public visionlab::Stage
{
public:
    std::string_view name() const noexcept override
    {
        return "add";
    }
    void process(visionlab::FrameResult& result) override
    {
        result.frame.image.data()[0] += 3;
    }
};

class MultiplyStage final : public visionlab::Stage
{
public:
    std::string_view name() const noexcept override
    {
        return "multiply";
    }
    void process(visionlab::FrameResult& result) override
    {
        result.frame.image.data()[0] *= 2;
    }
};

class ThrowStage final : public visionlab::Stage
{
public:
    std::string_view name() const noexcept override
    {
        return "failure";
    }
    void process(visionlab::FrameResult&) override
    {
        throw std::runtime_error("stage failure");
    }
};

void test_nearest_neighbor()
{
    rejects(
        []
        {
            visionlab::NearestNeighborStage invalid(0);
        });
    visionlab::NearestNeighborStage tracker(20);
    const auto run = [&](std::uint64_t index, std::vector<visionlab::BoxOverlay> boxes)
    {
        visionlab::FrameResult result{{index, std::nullopt, visionlab::Image(200, 100)}, {}, std::move(boxes), {}};
        tracker.process(result);
        return result.boxes;
    };
    const std::vector<visionlab::BoxOverlay> first = run(0, {{10, 10, 10, 10, ""}, {100, 10, 10, 10, ""}});
    require(first[0].label == "ID 1 (new)" && first[1].label == "ID 2 (new)", "Initial unique IDs");
    const std::vector<visionlab::BoxOverlay> reordered = run(1, {{98, 10, 10, 10, ""}, {12, 10, 10, 10, ""}});
    require(reordered[0].label == "ID 2 (matched)" && reordered[1].label == "ID 1 (matched)", "IDs follow position, not detection order");
    const std::vector<visionlab::BoxOverlay> competition = run(2, {{13, 10, 10, 10, ""}, {16, 10, 10, 10, ""}, {160, 10, 10, 10, ""}});
    require(competition[0].label == "ID 1 (matched)" && competition[1].label == "ID 3 (new)" && competition[2].label == "ID 4 (new)",
            "One-to-one matching and distance gate");
    require(run(3, {}).empty(), "Empty frame retires tracks");
    require(run(4, {{13, 10, 10, 10, ""}})[0].label == "ID 5 (new)", "Missing objects do not reuse retired IDs");
    require(run(0, {{13, 10, 10, 10, ""}})[0].label == "ID 1 (new)", "Rewinding resets temporal state");
}

} // namespace

int main()
{
    try
    {
        rejects(
            []
            {
                visionlab::Image invalid(0, 2);
            });
        rejects(
            []
            {
                visionlab::SyntheticSource invalid(2, -1, 10);
            });
        rejects(
            []
            {
                visionlab::SyntheticSource invalid(2, 10, 10, 0.0);
            });
        rejects(
            []
            {
                visionlab::SyntheticSource invalid(2, 10, 10, std::numeric_limits<double>::quiet_NaN());
            });
        visionlab::Pipeline empty;
        rejects(
            [&]
            {
                empty.add(nullptr);
            });
        visionlab::SyntheticSource source(2, 16, 8, 25.0);
        visionlab::SyntheticSource identical(2, 16, 8, 25.0);
        std::optional<visionlab::Frame> first = source.next();
        std::optional<visionlab::Frame> same = identical.next();
        require(first && same, "Synthetic source must yield its first frame");
        require(first->image.stride_bytes() == 48 && first->image.size_bytes() == 384, "BGR layout");
        require(std::memcmp(first->image.data(), same->image.data(), first->image.size_bytes()) == 0,
                "Synthetic source must be deterministic");
        const std::uint8_t first_pixel = first->image.data()[0];
        const visionlab::FrameResult unchanged = empty.process(std::move(*first));
        require(unchanged.frame.index == 0 && unchanged.frame.image.data()[0] == first_pixel, "Empty pipeline must preserve frame data");
        require(unchanged.timings.empty() && unchanged.snapshots.empty(), "Empty headless pipeline output");
        std::optional<visionlab::Frame> second = source.next();
        require(second && second->index == 1 && second->timestamp_seconds && std::abs(*second->timestamp_seconds - 0.04) < 1e-9,
                "Frame sequence/timestamps");
        require(!source.next() && !source.next(), "EOF must remain stable");
        visionlab::SyntheticSource zero(0);
        require(!zero.next(), "Empty source must immediately finish");

        visionlab::Pipeline ordered;
        ordered.add(std::make_unique<AddStage>());
        ordered.add(std::make_unique<MultiplyStage>());
        second->image.data()[0] = 10;
        visionlab::FrameResult result = ordered.process(std::move(*second), true);
        require(result.frame.image.data()[0] == 26, "Stages must run in insertion order");
        require(result.snapshots.size() == 3 && result.snapshots[1].image.data()[0] == 13,
                "Stage snapshots must own their pixels independently");
        require(result.timings.size() == 2 && result.timings[0].name == "add" && result.timings[1].name == "multiply" &&
                    result.timings[0].milliseconds >= 0,
                "Stage timings must describe the executed stages");

        visionlab::Pipeline failing;
        failing.add(std::make_unique<ThrowStage>());
        bool propagated = false;
        try
        {
            failing.process(std::move(result.frame));
        }
        catch (const std::runtime_error&)
        {
            propagated = true;
        }
        require(propagated, "A stage failure must reach the frontend");
        test_nearest_neighbor();
        std::cout << "Core contracts passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
