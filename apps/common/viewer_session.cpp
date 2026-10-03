#include "viewer_session.hpp"
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace visionlab::app
{

ViewerSession::ViewerSession(const PipelineCatalog& pipelines, std::uint64_t frame_limit, std::size_t history_byte_limit)
    : pipelines_(pipelines), frame_limit_(frame_limit), history_byte_limit_(history_byte_limit)
{
    if (frame_limit == 0)
    {
        throw std::invalid_argument("Frame limit must be positive");
    }
    if (history_byte_limit == 0)
    {
        throw std::invalid_argument("History byte limit must be positive");
    }
}

std::size_t ViewerSession::CachedFrame::pixel_bytes() const noexcept
{
    std::size_t bytes = raw.image.size_bytes();
    for (const std::optional<FrameResult>& result : processed)
    {
        if (result)
        {
            bytes += result->frame.image.size_bytes();
            for (const StageSnapshot& snapshot : result->snapshots)
            {
                bytes += snapshot.image.size_bytes();
            }
        }
    }
    return bytes;
}

bool ViewerSession::select_source(const SourceEntry& entry)
{
    try
    {
        std::unique_ptr<FrameSource> source = open_source(entry, frame_limit_);
        std::optional<Frame> raw = source->next();
        if (!raw)
        {
            throw std::runtime_error("The source contained no frames");
        }
        std::array<Pipeline, 2> pipelines;
        std::array<std::optional<FrameResult>, 2> results;
        for (std::size_t side = 0; side < (compare_mode_ ? 2u : 1u); ++side)
        {
            pipelines[side] = pipelines_.create(views_[side].id);
            results[side] = pipelines[side].process(*raw, true);
        }
        std::deque<CachedFrame> history;
        history.push_back({std::move(*raw), std::move(results), 0});
        // Commit only after opening and processing succeed; a bad selection preserves the current session.
        source_entry_ = entry;
        source_ = std::move(source);
        for (std::size_t side = 0; side < views_.size(); ++side)
        {
            views_[side].pipeline = std::move(pipelines[side]);
            views_[side].stage = 0;
            views_[side].start_position = 0;
        }
        history_ = std::move(history);
        history_index_ = 0;
        history_bytes_ = history_.front().pixel_bytes();
        frames_read_ = 1;
        playing_ = false;
        ended_ = entry.kind == SourceKind::Image || frames_read_ >= frame_limit_;
        error_.clear();
        ++revision_;
        return true;
    }
    catch (const std::exception& error)
    {
        error_ = error.what();
        return false;
    }
}

bool ViewerSession::select_pipeline(const std::string& id, std::size_t side)
{
    try
    {
        if (side >= views_.size())
        {
            throw std::invalid_argument("Invalid comparison side");
        }
        std::string selected_id = id;
        Pipeline pipeline = pipelines_.create(selected_id);
        const std::size_t other = 1 - side;
        const bool other_active = other == 0 || compare_mode_;
        Pipeline restored_other;
        std::deque<CachedFrame> history;
        std::unique_ptr<FrameSource> repositioned_source;
        std::uint64_t frames_read = frames_read_;
        std::uint64_t start_position = 0;
        if (!history_.empty())
        {
            const CachedFrame& current = history_[history_index_];
            CachedFrame replacement = current;
            replacement.processed[side] = pipeline.process(current.raw, true);
            history.push_back(std::move(replacement));
            start_position = current.source_position;
            if (history_index_ + 1 < history_.size())
            {
                // Reposition decoding and restore only the unchanged pipeline's temporal state.
                // The newly selected pipeline starts fresh at the displayed frame.
                repositioned_source = open_source(source_entry_, frame_limit_);
                if (other_active)
                {
                    restored_other = pipelines_.create(views_[other].id);
                }
                frames_read = current.source_position + 1;
                for (std::uint64_t index = 0; index < frames_read; ++index)
                {
                    std::optional<Frame> replay = repositioned_source->next();
                    if (!replay)
                    {
                        throw std::runtime_error("Cannot reposition source to the displayed frame");
                    }
                    if (other_active && index >= views_[other].start_position)
                    {
                        restored_other.process(std::move(*replay));
                    }
                }
            }
        }
        views_[side].id = std::move(selected_id);
        views_[side].pipeline = std::move(pipeline);
        views_[side].start_position = start_position;
        if (repositioned_source)
        {
            source_ = std::move(repositioned_source);
            if (other_active)
            {
                views_[other].pipeline = std::move(restored_other);
            }
            frames_read_ = frames_read;
            ended_ = frames_read_ >= frame_limit_;
        }
        history_ = std::move(history);
        history_index_ = 0;
        history_bytes_ = history_.empty() ? 0 : history_.front().pixel_bytes();
        views_[side].stage = 0;
        compare_mode_ = compare_mode_ || side == 1;
        playing_ = false;
        error_.clear();
        ++revision_;
        return true;
    }
    catch (const std::exception& error)
    {
        error_ = error.what();
        return false;
    }
}

bool ViewerSession::set_compare_mode(bool enabled)
{
    if (enabled == compare_mode_)
    {
        return true;
    }
    if (enabled)
    {
        const std::size_t stage = views_[1].stage;
        if (!select_pipeline(views_[1].id, 1))
        {
            return false;
        }
        if (result(1))
        {
            views_[1].stage = std::min(stage, result(1)->snapshots.size() - 1);
        }
        return true;
    }
    // Release right-side results while preserving left-side history and processing state.
    compare_mode_ = false;
    views_[1].pipeline = Pipeline{};
    history_bytes_ = 0;
    for (CachedFrame& frame : history_)
    {
        frame.processed[1].reset();
        history_bytes_ += frame.pixel_bytes();
    }
    playing_ = false;
    error_.clear();
    ++revision_;
    return true;
}

bool ViewerSession::compare_mode() const noexcept
{
    return compare_mode_;
}

bool ViewerSession::restart()
{
    const std::array<std::size_t, 2> selected_stages{views_[0].stage, views_[1].stage};
    if (!select_source(source_entry_))
    {
        return false;
    }
    for (std::size_t side = 0; side < (compare_mode_ ? 2u : 1u); ++side)
    {
        views_[side].stage = std::min(selected_stages[side], result(side)->snapshots.size() - 1);
    }
    return true;
}

void ViewerSession::select_cached_frame(std::size_t index)
{
    history_index_ = index;
    for (std::size_t side = 0; side < (compare_mode_ ? 2u : 1u); ++side)
    {
        views_[side].stage = std::min(views_[side].stage, result(side)->snapshots.size() - 1);
    }
    ++revision_;
}

bool ViewerSession::previous_frame()
{
    playing_ = false;
    if (!can_previous_frame())
    {
        return false;
    }
    select_cached_frame(history_index_ - 1);
    return true;
}

bool ViewerSession::next_frame()
{
    if (history_index_ + 1 < history_.size())
    {
        select_cached_frame(history_index_ + 1);
        if (ended())
        {
            playing_ = false;
        }
        return true;
    }
    if (!can_next_frame())
    {
        return false;
    }
    try
    {
        std::optional<Frame> raw = source_->next();
        if (!raw)
        {
            ended_ = true;
            playing_ = false;
            return false;
        }
        std::array<std::optional<FrameResult>, 2> results;
        for (std::size_t side = 0; side < (compare_mode_ ? 2u : 1u); ++side)
        {
            results[side] = views_[side].pipeline.process(*raw, true);
        }
        // Publish a frame only after both pipelines succeed, so panes never show different frames.
        history_.push_back({std::move(*raw), std::move(results), frames_read_});
        history_bytes_ += history_.back().pixel_bytes();
        // Always retain two frames so one backward step remains possible, even for oversized images.
        while (history_.size() > 2 && history_bytes_ > history_byte_limit_)
        {
            history_bytes_ -= history_.front().pixel_bytes();
            history_.pop_front();
        }
        select_cached_frame(history_.size() - 1);
        ++frames_read_;
        ended_ = frames_read_ >= frame_limit_;
        if (ended_)
        {
            playing_ = false;
        }
        error_.clear();
        return true;
    }
    catch (const std::exception& error)
    {
        error_ = std::string(error.what()) + " Restart or select another source to continue.";
        playing_ = false;
        ended_ = true;
        return false;
    }
}

void ViewerSession::select_stage(std::size_t index, std::size_t side)
{
    if (result(side) && index < result(side)->snapshots.size())
    {
        playing_ = false;
        views_[side].stage = index;
    }
}

void ViewerSession::set_playing(bool playing) noexcept
{
    playing_ = playing && can_next_frame();
}

bool ViewerSession::playing() const noexcept
{
    return playing_;
}
bool ViewerSession::ended() const noexcept
{
    return ended_ && history_index_ + 1 == history_.size();
}
bool ViewerSession::can_previous_frame() const noexcept
{
    return history_index_ > 0;
}
bool ViewerSession::can_next_frame() const noexcept
{
    return history_index_ + 1 < history_.size() || (source_ && !ended_);
}
const std::string& ViewerSession::error() const noexcept
{
    return error_;
}
const SourceEntry& ViewerSession::source_entry() const noexcept
{
    return source_entry_;
}
const std::string& ViewerSession::pipeline_id(std::size_t side) const
{
    return views_.at(side).id;
}
const FrameResult* ViewerSession::result(std::size_t side) const noexcept
{
    if (history_.empty() || side >= views_.size() || (side == 1 && !compare_mode_))
    {
        return nullptr;
    }
    const std::optional<FrameResult>& processed = history_[history_index_].processed[side];
    return processed ? &*processed : nullptr;
}
const StageSnapshot* ViewerSession::snapshot(std::size_t side) const noexcept
{
    return result(side) ? &result(side)->snapshots[views_[side].stage] : nullptr;
}
std::size_t ViewerSession::stage_index(std::size_t side) const
{
    return views_.at(side).stage;
}
std::uint64_t ViewerSession::revision() const noexcept
{
    return revision_;
}

} // namespace visionlab::app
