#pragma once

#include "catalogs.hpp"
#include <array>
#include <cstddef>
#include <deque>

namespace visionlab::app
{

// UI-independent interaction state. Browsing cached frames/stages never executes processing.
class ViewerSession
{
public:
    explicit ViewerSession(const PipelineCatalog& pipelines, std::uint64_t frame_limit = 300,
                           std::size_t history_byte_limit = 256 * 1024 * 1024);
    bool select_source(const SourceEntry& entry);
    bool select_pipeline(const std::string& id, std::size_t side = 0);
    bool set_compare_mode(bool enabled);
    bool compare_mode() const noexcept;
    bool restart();
    bool previous_frame();
    bool next_frame();
    void select_stage(std::size_t index, std::size_t side = 0);
    void set_playing(bool playing) noexcept;

    bool playing() const noexcept;
    bool ended() const noexcept;
    bool can_previous_frame() const noexcept;
    bool can_next_frame() const noexcept;
    const std::string& error() const noexcept;
    const SourceEntry& source_entry() const noexcept;
    const std::string& pipeline_id(std::size_t side = 0) const;
    const FrameResult* result(std::size_t side = 0) const noexcept;
    const StageSnapshot* snapshot(std::size_t side = 0) const noexcept;
    std::size_t stage_index(std::size_t side = 0) const;
    std::uint64_t revision() const noexcept;

private:
    struct CachedFrame
    {
        Frame raw;
        std::array<std::optional<FrameResult>, 2> processed;
        std::uint64_t source_position;
        std::size_t pixel_bytes() const noexcept;
    };

    struct PipelineView
    {
        std::string id = "pass-through";
        Pipeline pipeline;
        std::size_t stage = 0;
        std::uint64_t start_position = 0;
    };

    void select_cached_frame(std::size_t index);
    const PipelineCatalog& pipelines_;
    std::uint64_t frame_limit_;
    std::size_t history_byte_limit_;
    SourceEntry source_entry_{"Synthetic pattern", {}, SourceKind::Synthetic};
    std::unique_ptr<FrameSource> source_;
    std::array<PipelineView, 2> views_;
    std::deque<CachedFrame> history_;
    std::size_t history_index_ = 0;
    std::size_t history_bytes_ = 0;
    std::string error_;
    std::uint64_t frames_read_ = 0;
    std::uint64_t revision_ = 0;
    bool playing_ = false;
    bool ended_ = false;
    bool compare_mode_ = false;
};

} // namespace visionlab::app
