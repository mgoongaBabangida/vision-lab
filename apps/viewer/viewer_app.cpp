#include "viewer_app.hpp"
#include "graphics.hpp"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>

namespace visionlab::app
{
namespace
{

void draw_pipeline_controls(ViewerSession& session, const PipelineCatalog& pipelines, std::size_t side)
{
    ImGui::PushID(static_cast<int>(side));
    const char* pipeline_label = session.pipeline_id(side).c_str();
    for (const PipelineDefinition& definition : pipelines.entries())
    {
        if (definition.id == session.pipeline_id(side))
        {
            pipeline_label = definition.label.c_str();
        }
    }
    ImGui::SetNextItemWidth(-1);
    if (ImGui::BeginCombo("##Pipeline", pipeline_label))
    {
        for (const PipelineDefinition& definition : pipelines.entries())
        {
            if (ImGui::Selectable(definition.label.c_str(), definition.id == session.pipeline_id(side)))
            {
                session.select_pipeline(definition.id, side);
            }
        }
        ImGui::EndCombo();
    }
    const FrameResult* result = session.result(side);
    ImGui::BeginDisabled(!result || session.stage_index(side) == 0);
    if (ImGui::Button("Previous stage"))
    {
        session.select_stage(session.stage_index(side) - 1, side);
    }
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!result || session.stage_index(side) + 1 >= result->snapshots.size());
    if (ImGui::Button("Next stage"))
    {
        session.select_stage(session.stage_index(side) + 1, side);
    }
    ImGui::EndDisabled();
    ImGui::SetNextItemWidth(-1);
    if (result && ImGui::BeginCombo("##Stage", session.snapshot(side)->name.c_str()))
    {
        for (std::size_t index = 0; index < result->snapshots.size(); ++index)
        {
            ImGui::PushID(static_cast<int>(index));
            if (ImGui::Selectable(result->snapshots[index].name.c_str(), index == session.stage_index(side)))
            {
                session.select_stage(index, side);
            }
            ImGui::PopID();
        }
        ImGui::EndCombo();
    }
    ImGui::PopID();
}

} // namespace

ViewerOptions parse_viewer_options(int argc, char** argv)
{
    ViewerOptions options;
    options.source_folder = std::filesystem::u8path(VISIONLAB_DEFAULT_SOURCE_DIR);
    std::vector<char*> common_arguments{argv[0]};
    for (int index = 1; index < argc; ++index)
    {
        const std::string argument(argv[index]);
        if (argument == "--source-dir" || argument == "--capture")
        {
            if (++index == argc)
            {
                throw std::invalid_argument(argument + " requires a path");
            }
            if (argument == "--source-dir")
            {
                options.source_folder = std::filesystem::u8path(argv[index]);
            }
            else
            {
                options.capture_path = std::filesystem::u8path(argv[index]);
            }
        }
        else if (argument == "--compare-pipeline")
        {
            if (++index == argc)
            {
                throw std::invalid_argument("--compare-pipeline requires a pipeline ID");
            }
            options.compare_pipeline = argv[index];
        }
        else if (argument == "--smoke-test")
        {
            options.smoke_test = true;
        }
        else
        {
            common_arguments.push_back(argv[index]);
        }
    }
    options.processing = parse_options(static_cast<int>(common_arguments.size()), common_arguments.data());
    if (!options.smoke_test && !options.capture_path.empty())
    {
        throw std::invalid_argument("--capture is only available with --smoke-test");
    }
    return options;
}

ViewerApp::ViewerApp(ViewerOptions options) : options_(std::move(options))
{
}

int ViewerApp::run()
{
    if (options_.processing.help)
    {
        std::cout << usage("visionlab_viewer") << "Viewer: [--source-dir folder] [--compare-pipeline ID]\n"
                  << "Space: play/pause. P/N: previous/next frame. Left/Right: stage. Esc: quit.\n"
                  << "Starts paused. EOF keeps the window open. Pipeline changes reset stage history.\n";
        return 0;
    }
    PipelineCatalog pipelines;
    ViewerSession session(pipelines, options_.processing.frames);
    if (!session.select_pipeline(options_.processing.pipeline))
    {
        throw std::runtime_error(session.error());
    }
    std::vector<SourceEntry> sources{{"Synthetic pattern", {}, SourceKind::Synthetic}};
    std::array<char, 4096> folder_buffer{};
    const std::string initial_folder = std::filesystem::absolute(options_.source_folder).u8string();
    if (initial_folder.size() >= folder_buffer.size())
    {
        throw std::runtime_error("Source-folder path is too long");
    }
    std::copy(initial_folder.begin(), initial_folder.end(), folder_buffer.begin());
    std::string folder_error;
    const auto refresh = [&]
    {
        try
        {
            sources = SourceCatalog::scan(std::filesystem::u8path(folder_buffer.data()));
            folder_error.clear();
        }
        catch (const std::exception& error)
        {
            folder_error = error.what();
        }
    };
    refresh();
    if (!session.select_source(sources.front()))
    {
        throw std::runtime_error(session.error());
    }
    if (!options_.processing.input.empty())
    {
        if (!session.select_source(SourceCatalog::from_path(std::filesystem::u8path(options_.processing.input))))
        {
            throw std::runtime_error(session.error());
        }
    }
    if (!options_.compare_pipeline.empty())
    {
        if (!session.select_pipeline(options_.compare_pipeline, 1))
        {
            throw std::runtime_error(session.error());
        }
        session.select_stage(session.result()->snapshots.size() - 1);
        session.select_stage(session.result(1)->snapshots.size() - 1, 1);
    }
    Graphics graphics;
    graphics.initialize(options_.smoke_test);
    bool running = true;
    int rendered_frames = 0;
    std::chrono::steady_clock::time_point next_frame_at = std::chrono::steady_clock::now();
    while (running)
    {
        const std::chrono::steady_clock::time_point loop_start = std::chrono::steady_clock::now();
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT || (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE))
            {
                running = false;
            }
            if (event.type == SDL_KEYDOWN && !event.key.repeat && !ImGui::GetIO().WantCaptureKeyboard)
            {
                switch (event.key.keysym.sym)
                {
                case SDLK_ESCAPE:
                    running = false;
                    break;
                case SDLK_SPACE:
                    session.set_playing(!session.playing());
                    break;
                case SDLK_n:
                    session.set_playing(false);
                    session.next_frame();
                    break;
                case SDLK_p:
                    session.previous_frame();
                    break;
                case SDLK_LEFT:
                    if (session.stage_index() > 0)
                    {
                        session.select_stage(session.stage_index() - 1);
                    }
                    break;
                case SDLK_RIGHT:
                    session.select_stage(session.stage_index() + 1);
                    break;
                default:
                    break;
                }
            }
        }
        if (!running)
        {
            break;
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Vision Lab", nullptr,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoSavedSettings);
        ImGui::TextUnformatted("VISION LAB  /  Pipeline inspector");
        ImGui::Separator();
        ImGui::SetNextItemWidth(std::max(200.0f, ImGui::GetContentRegionAvail().x - 230.0f));
        const bool folder_entered =
            ImGui::InputText("Source folder", folder_buffer.data(), folder_buffer.size(), ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        if (ImGui::Button("Refresh") || folder_entered)
        {
            refresh();
        }
        if (!folder_error.empty())
        {
            ImGui::TextWrapped("Folder: %s", folder_error.c_str());
        }
        ImGui::SetNextItemWidth(std::max(200.0f, ImGui::GetContentRegionAvail().x - 130.0f));
        if (ImGui::BeginCombo("Source", session.source_entry().label.c_str()))
        {
            for (const SourceEntry& entry : sources)
            {
                const bool selected = entry.kind == session.source_entry().kind && entry.path == session.source_entry().path;
                if (ImGui::Selectable(entry.label.c_str(), selected))
                {
                    session.select_source(entry);
                }
                if (selected)
                {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        if (!SourceCatalog::media_available())
        {
            ImGui::TextUnformatted("File input is unavailable in this build; synthetic input is ready.");
        }
        bool compare = session.compare_mode();
        if (ImGui::Checkbox("Compare mode", &compare))
        {
            session.set_compare_mode(compare);
        }
        ImGui::BeginDisabled(!session.can_next_frame());
        if (ImGui::Button(session.playing() ? "Pause" : "Play"))
        {
            session.set_playing(!session.playing());
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::BeginDisabled(!session.can_previous_frame());
        if (ImGui::Button("Previous frame"))
        {
            session.previous_frame();
        }
        ImGui::EndDisabled();
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        {
            ImGui::SetTooltip("Previous cached frame (P). Pauses playback and preserves the selected stage.\n"
                              "Disabled at the oldest retained frame. Restart returns to frame zero.\n"
                              "History uses a 256 MiB pixel budget, retaining at least two frames.");
        }
        ImGui::SameLine();
        ImGui::BeginDisabled(!session.can_next_frame());
        if (ImGui::Button("Next frame"))
        {
            session.set_playing(false);
            session.next_frame();
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Restart"))
        {
            session.restart();
        }
        ImGui::SameLine();
        ImGui::TextUnformatted(session.ended() ? "End of source / frame limit" : session.playing() ? "Playing" : "Paused");
        ImGui::Separator();
        const int pane_count = session.compare_mode() ? 2 : 1;
        if (ImGui::BeginTable("Pipeline controls", pane_count, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV))
        {
            for (int pane = 0; pane < pane_count; ++pane)
            {
                ImGui::TableNextColumn();
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted(session.compare_mode() ? (pane == 0 ? "Left pipeline" : "Right pipeline") : "Pipeline");
                draw_pipeline_controls(session, pipelines, static_cast<std::size_t>(pane));
            }
            ImGui::EndTable();
        }
        // Handle controls before playback: inspecting a stage freezes this frame immediately.
        if (session.playing() && loop_start >= next_frame_at)
        {
            session.next_frame();
            next_frame_at = loop_start + std::chrono::milliseconds(33);
        }
        if (!session.error().empty())
        {
            ImGui::TextWrapped("Source / pipeline: %s", session.error().c_str());
        }
        ImGui::Separator();
        if (ImGui::BeginTable("Pipeline images", pane_count, ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_BordersInnerV))
        {
            for (int pane = 0; pane < pane_count; ++pane)
            {
                ImGui::TableNextColumn();
                ImGui::PushID(pane);
                const std::size_t side = static_cast<std::size_t>(pane);
                const FrameResult* result = session.result(side);
                if (result)
                {
                    const StageSnapshot& snapshot = *session.snapshot(side);
                    ImGui::TextWrapped("Frame %llu | View %zu/%zu | %d x %d | %.3f ms",
                                       static_cast<unsigned long long>(result->frame.index), session.stage_index(side) + 1,
                                       result->snapshots.size(), snapshot.image.width(), snapshot.image.height(), snapshot.milliseconds);
                }
                const float image_height =
                    ImGui::GetContentRegionAvail().y - ImGui::GetStyle().ItemSpacing.y - 2.0f * ImGui::GetStyle().CellPadding.y;
                ImGui::BeginChild("Image", ImVec2(0, std::max(50.0f, image_height)), false,
                                  ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                graphics.draw_image(session, side);
                ImGui::EndChild();
                ImGui::PopID();
            }
            ImGui::EndTable();
        }
        ImGui::End();
        ImGui::Render();
        ++rendered_frames;
        graphics.present(options_.smoke_test && rendered_frames == 3 ? options_.capture_path : std::filesystem::path{});
        if (options_.smoke_test && rendered_frames >= 3)
        {
            running = false;
        }
        // Avoid spinning on drivers without vsync. Playback is inspection-paced, not media-synchronized.
        std::this_thread::sleep_until(loop_start + std::chrono::milliseconds(16));
    }
    return 0;
}

} // namespace visionlab::app
