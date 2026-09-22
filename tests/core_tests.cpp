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

namespace {

void require(bool condition, const char* message) {
    if (!condition) { throw std::runtime_error(message); }
}

template <typename Function>
void rejects(Function function) {
    bool thrown = false;
    try { function(); } catch (const std::invalid_argument&) { thrown = true; }
    require(thrown, "Expected invalid_argument");
}

class AddStage final : public visionlab::Stage {
public:
    std::string_view name() const noexcept override { return "add"; }
    void process(visionlab::FrameResult& result) override {
        result.frame.image.data()[0] += 3;
        result.debug_images.push_back({"after-add", result.frame.image});
    }
};

class MultiplyStage final : public visionlab::Stage {
public:
    std::string_view name() const noexcept override { return "multiply"; }
    void process(visionlab::FrameResult& result) override {
        result.frame.image.data()[0] *= 2;
    }
};

class ThrowStage final : public visionlab::Stage {
public:
    std::string_view name() const noexcept override { return "failure"; }
    void process(visionlab::FrameResult&) override { throw std::runtime_error("stage failure"); }
};

} // namespace

int main() {
    try {
        rejects([] { visionlab::Image invalid(0, 2); });
        rejects([] { visionlab::SyntheticSource invalid(2, -1, 10); });
        rejects([] { visionlab::SyntheticSource invalid(2, 10, 10, 0.0); });
        rejects([] { visionlab::SyntheticSource invalid(2, 10, 10, std::numeric_limits<double>::quiet_NaN()); });
        visionlab::Pipeline empty;
        rejects([&] { empty.add(nullptr); });
        visionlab::SyntheticSource source(2, 16, 8, 25.0);
        visionlab::SyntheticSource identical(2, 16, 8, 25.0);
        auto first = source.next();
        auto same = identical.next();
        require(first && same, "Synthetic source must yield its first frame");
        require(first->image.stride_bytes() == 48 && first->image.size_bytes() == 384, "BGR layout");
        require(std::memcmp(first->image.data(), same->image.data(), first->image.size_bytes()) == 0,
                "Synthetic source must be deterministic");
        const auto first_pixel = first->image.data()[0];
        const auto unchanged = empty.process(std::move(*first));
        require(unchanged.frame.index == 0 && unchanged.frame.image.data()[0] == first_pixel,
                "Empty pipeline must preserve frame data");
        require(unchanged.timings.empty() && unchanged.debug_images.empty(), "Empty pipeline output");
        auto second = source.next();
        require(second && second->index == 1 && second->timestamp_seconds
                && std::abs(*second->timestamp_seconds - 0.04) < 1e-9, "Frame sequence/timestamps");
        require(!source.next() && !source.next(), "EOF must remain stable");
        visionlab::SyntheticSource zero(0);
        require(!zero.next(), "Empty source must immediately finish");

        visionlab::Pipeline ordered;
        ordered.add(std::make_unique<AddStage>());
        ordered.add(std::make_unique<MultiplyStage>());
        second->image.data()[0] = 10;
        auto result = ordered.process(std::move(*second));
        require(result.frame.image.data()[0] == 26, "Stages must run in insertion order");
        require(result.debug_images.size() == 1 && result.debug_images[0].image.data()[0] == 13,
                "Debug snapshots must own their pixels independently");
        require(result.timings.size() == 2 && result.timings[0].name == "add"
                && result.timings[1].name == "multiply" && result.timings[0].milliseconds >= 0,
                "Stage timings must describe the executed stages");

        visionlab::Pipeline failing;
        failing.add(std::make_unique<ThrowStage>());
        bool propagated = false;
        try { failing.process(std::move(result.frame)); }
        catch (const std::runtime_error&) { propagated = true; }
        require(propagated, "A stage failure must reach the frontend");
        std::cout << "Core contracts passed\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
