#include "viewer_session.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <stdexcept>

namespace
{

void require(bool condition, const char* message)
{
    if (!condition)
    {
        throw std::runtime_error(message);
    }
}

class CountingStage final : public visionlab::Stage
{
public:
    CountingStage(std::shared_ptr<int> calls, std::uint8_t increment) : calls_(std::move(calls)), increment_(increment)
    {
    }
    std::string_view name() const noexcept override
    {
        return "test-stage";
    }
    void process(visionlab::FrameResult& result) override
    {
        ++*calls_;
        result.frame.image.data()[0] += increment_;
        result.boxes.push_back({1, 2, 3, 4, std::to_string(++frames_seen_)});
    }

private:
    std::shared_ptr<int> calls_;
    std::uint8_t increment_;
    int frames_seen_ = 0;
};

void test_frame_history(const visionlab::app::PipelineCatalog& catalog, const std::shared_ptr<int>& calls)
{
    visionlab::app::ViewerSession session(catalog, 5);
    require(!session.can_previous_frame() && !session.can_next_frame(), "Empty session cannot navigate frames");
    require(session.select_source({"Synthetic", {}, visionlab::app::SourceKind::Synthetic}), "Open history source");
    require(!session.previous_frame() && session.result()->frame.index == 0, "Cannot move before frame zero");
    require(session.select_pipeline("test-two-stages"), "Select history test pipeline");
    require(session.next_frame() && session.next_frame(), "Process frames one and two");
    session.select_stage(2);
    const int processed_calls = *calls;
    const std::uint64_t revision = session.revision();
    session.set_playing(true);
    require(session.previous_frame() && !session.playing() && session.result()->frame.index == 1 && session.stage_index() == 2,
            "Previous frame pauses and preserves the selected stage");
    require(session.revision() > revision && session.snapshot()->boxes[0].label == "2", "Historical image refreshes with original state");
    require(session.next_frame() && session.result()->frame.index == 2 && *calls == processed_calls,
            "Forward through history neither decodes nor runs stages again");
    require(session.next_frame() && session.result()->frame.index == 3 && session.snapshot()->boxes[0].label == "4",
            "New processing continues from the pipeline's original temporal state");
    require(session.next_frame() && session.ended(), "Reach frame limit");
    require(session.previous_frame() && !session.ended() && session.can_next_frame(), "Backward navigation works at the frame limit");
    const int calls_at_limit = *calls;
    session.set_playing(true);
    require(session.playing() && session.next_frame() && session.ended() && !session.playing() && *calls == calls_at_limit,
            "Play replays cached frames and pauses at the exhausted frontier");
    require(session.previous_frame() && session.previous_frame(), "Return to frame two");
    require(!session.select_pipeline("test-failure") && session.can_previous_frame() && session.can_next_frame() &&
                session.result()->frame.index == 2,
            "Failed pipeline switch preserves history and its cursor");
    require(session.select_pipeline("test-two-stages") && !session.can_previous_frame() && session.snapshot()->name == "Source" &&
                session.result()->frame.index == 2 && session.result()->boxes[0].label == "1" && *calls == calls_at_limit + 2,
            "Pipeline switch clears old history and processes only the displayed raw frame with fresh stages");
    require(session.next_frame() && session.result()->frame.index == 3 && session.result()->boxes[0].label == "2",
            "Pipeline switch in history repositions decoding to the following frame");
    require(session.restart() && !session.can_previous_frame() && session.result()->frame.index == 0,
            "Restart clears history and pipeline state");
    require(session.next_frame() && session.select_source({"Another synthetic", {}, visionlab::app::SourceKind::Synthetic}) &&
                !session.can_previous_frame() && session.result()->frame.index == 0,
            "Source selection clears old history");

    visionlab::app::ViewerSession bounded(catalog, 5, 1);
    require(bounded.select_source({"Synthetic", {}, visionlab::app::SourceKind::Synthetic}) && bounded.next_frame() && bounded.next_frame(),
            "Read beyond a deliberately tiny history budget");
    require(bounded.previous_frame() && bounded.result()->frame.index == 1 && !bounded.can_previous_frame() && !bounded.previous_frame(),
            "Budget evicts oldest frames while retaining at least the current and previous frame");
    require(bounded.next_frame() && bounded.result()->frame.index == 2 && bounded.next_frame() && bounded.result()->frame.index == 3,
            "Forward browsing after eviction resumes at the next undecoded frame");
}

// The test owns only this newly created directory; never reuse or delete user data.
struct FixtureDirectory
{
    std::filesystem::path path = std::filesystem::current_path() / "visionlab-catalog-fixtures";
    FixtureDirectory()
    {
        if (!std::filesystem::create_directory(path))
        {
            throw std::runtime_error("Fixture directory already exists");
        }
    }
    ~FixtureDirectory()
    {
        std::error_code ignored;
        std::filesystem::remove_all(path, ignored);
    }
};

} // namespace

int main()
{
    try
    {
        const auto calls = std::make_shared<int>(0);
        visionlab::app::PipelineCatalog catalog;
        catalog.add({"test-two-stages", "Test only", [calls]
                     {
                         visionlab::Pipeline pipeline;
                         pipeline.add(std::make_unique<CountingStage>(calls, std::uint8_t{1}));
                         pipeline.add(std::make_unique<CountingStage>(calls, std::uint8_t{5}));
                         return pipeline;
                     }});
        catalog.add({"test-failure", "Test failure", []() -> visionlab::Pipeline
                     {
                         throw std::runtime_error("Factory failure");
                     }});
        test_frame_history(catalog, calls);
        *calls = 0;
        visionlab::app::ViewerSession session(catalog, 3);
        require(session.select_source({"Synthetic", {}, visionlab::app::SourceKind::Synthetic}), "Initial source opens");
        require(!session.playing() && session.result()->frame.index == 0, "Source opens paused at the first frame");
        require(session.result()->snapshots.size() == 1 && session.snapshot()->name == "Source", "Pass-through source snapshot");
        require(session.select_pipeline("test-two-stages"), "Select registered pipeline");
        require(*calls == 2 && session.result()->snapshots.size() == 3, "Exactly one execution of each stage");
        require(session.result()->snapshots[0].image.data()[0] == 0 && session.result()->snapshots[1].image.data()[0] == 1 &&
                    session.result()->snapshots[2].image.data()[0] == 6,
                "Independent ordered snapshots");
        require(session.result()->snapshots[0].boxes.empty() && session.result()->snapshots[1].boxes.size() == 1 &&
                    session.result()->snapshots[2].boxes.size() == 2,
                "Overlays belong to their stage snapshot");
        session.set_playing(true);
        const std::uint64_t revision = session.revision();
        session.select_stage(2);
        session.select_stage(0);
        session.select_stage(1);
        require(!session.playing() && *calls == 2 && session.revision() == revision && session.result()->frame.index == 0,
                "Stage browsing pauses without executing stages or advancing the source");
        require(session.next_frame() && *calls == 4 && session.result()->frame.index == 1, "Next frame executes stages once");
        require(session.stage_index() == 1, "Selected stage persists across frames");
        require(session.select_pipeline("pass-through"), "Switch back to pass-through");
        require(session.result()->frame.index == 1 && session.result()->frame.image.data()[0] == 0,
                "Pipeline switch reprocesses the retained original, not a previously processed image");
        require(!session.select_pipeline("test-failure") && session.pipeline_id() == "pass-through" && session.result()->frame.index == 1,
                "Failed pipeline switch preserves the session");
        require(!session.select_pipeline("missing") && session.pipeline_id() == "pass-through", "Unknown pipeline preserves session");
        require(!session.select_source({"Missing", "nonexistent-visionlab-file.png", visionlab::app::SourceKind::Image}) &&
                    session.result()->frame.index == 1 && session.source_entry().kind == visionlab::app::SourceKind::Synthetic,
                "Failed source selection preserves active source and image");
        require(session.next_frame() && session.ended() && session.result()->frame.index == 2, "Frame limit preserves last frame");
        require(!session.next_frame() && session.result()->frame.index == 2, "EOF does not clear the result");
        session.set_playing(true);
        require(!session.playing(), "Playback stays paused at the end");
        require(session.restart() && !session.ended() && session.result()->frame.index == 0, "Restart returns to frame zero");

        visionlab::Pipeline headless = catalog.create("test-two-stages");
        visionlab::SyntheticSource synthetic(1, 8, 8);
        const visionlab::FrameResult uncaptured = headless.process(*synthetic.next());
        require(uncaptured.snapshots.empty() && uncaptured.timings.size() == 2, "Headless mode avoids snapshot copies");

        const FixtureDirectory fixtures;
        std::ofstream(fixtures.path / "b.MP4").put('x');
        std::ofstream(fixtures.path / "a.PNG").put('x');
        std::ofstream(fixtures.path / "notes.txt").put('x');
        std::filesystem::create_directory(fixtures.path / "folder.jpg");
        const std::vector<visionlab::app::SourceEntry> entries = visionlab::app::SourceCatalog::scan(fixtures.path);
        require(entries.front().kind == visionlab::app::SourceKind::Synthetic, "Synthetic source is always present");
        if (visionlab::app::SourceCatalog::media_available())
        {
            require(entries.size() == 3 && entries[1].label == "a.PNG" && entries[2].label == "b.MP4",
                    "Catalog filters, sorts, ignores directories, and accepts uppercase extensions");
            require(!session.select_source(entries[1]), "Invalid image data is rejected at open time");
        }
        else
        {
            require(entries.size() == 1, "Non-OpenCV build lists only available sources");
        }
        std::cout << "Viewer session contracts passed\n";
        return 0;
    }
    catch (const std::exception& error)
    {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
