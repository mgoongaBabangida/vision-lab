#pragma once

#include "catalogs.hpp"
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
    bool select_pipeline(const std::string& id);
    bool restart();
    bool previous_frame();
    bool next_frame();
    void select_stage(std::size_t index);
    void set_playing(bool playing) noexcept;

    bool playing() const noexcept;
    bool ended() const noexcept;
    bool can_previous_frame() const noexcept;
    bool can_next_frame() const noexcept;
    const std::string& error() const noexcept;
    const SourceEntry& source_entry() const noexcept;
    const std::string& pipeline_id() const noexcept;
    const FrameResult* result() const noexcept;
    const StageSnapshot* snapshot() const noexcept;
    std::size_t stage_index() const noexcept;
    std::uint64_t revision() const noexcept;

private:
    struct CachedFrame
    {
        Frame raw;
        FrameResult processed;
        std::uint64_t source_position;
        std::size_t pixel_bytes() const noexcept;
    };

    void select_cached_frame(std::size_t index);
    const PipelineCatalog& pipelines_;
    std::uint64_t frame_limit_;
    std::size_t history_byte_limit_;
    SourceEntry source_entry_{"Synthetic pattern", {}, SourceKind::Synthetic};
    std::string pipeline_id_ = "pass-through";
    std::unique_ptr<FrameSource> source_;
    Pipeline pipeline_;
    std::deque<CachedFrame> history_;
    std::size_t history_index_ = 0;
    std::size_t history_bytes_ = 0;
    std::string error_;
    std::size_t stage_index_ = 0;
    std::uint64_t frames_read_ = 0;
    std::uint64_t revision_ = 0;
    bool playing_ = false;
    bool ended_ = false;
};

} // namespace visionlab::app
