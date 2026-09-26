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
    std::size_t bytes = raw.image.size_bytes() + processed.frame.image.size_bytes();
    for (const StageSnapshot& snapshot : processed.snapshots)
    {
        bytes += snapshot.image.size_bytes();
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
        Pipeline pipeline = pipelines_.create(pipeline_id_);
        FrameResult result = pipeline.process(*raw, true);
        std::deque<CachedFrame> history;
        history.push_back({std::move(*raw), std::move(result), 0});
        // Commit only after opening and processing succeed; a bad selection preserves the current session.
        source_entry_ = entry;
        source_ = std::move(source);
        pipeline_ = std::move(pipeline);
        history_ = std::move(history);
        history_index_ = 0;
        history_bytes_ = history_.front().pixel_bytes();
        stage_index_ = 0;
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

bool ViewerSession::select_pipeline(const std::string& id)
{
    try
    {
        Pipeline pipeline = pipelines_.create(id);
        std::deque<CachedFrame> history;
        std::unique_ptr<FrameSource> repositioned_source;
        std::uint64_t frames_read = frames_read_;
        if (!history_.empty())
        {
            const CachedFrame& current = history_[history_index_];
            FrameResult result = pipeline.process(current.raw, true);
            history.push_back({current.raw, std::move(result), current.source_position});
            if (history_index_ + 1 < history_.size())
            {
                // Source decoding is forward-only. Reopen only when switching pipelines in the past.
                // Decode up to the displayed frame, without running stages on those earlier frames.
                repositioned_source = open_source(source_entry_, frame_limit_);
                frames_read = current.source_position + 1;
                for (std::uint64_t index = 0; index < frames_read; ++index)
                {
                    if (!repositioned_source->next())
                    {
                        throw std::runtime_error("Cannot reposition source to the displayed frame");
                    }
                }
            }
        }
        pipeline_id_ = id;
        pipeline_ = std::move(pipeline);
        if (repositioned_source)
        {
            source_ = std::move(repositioned_source);
            frames_read_ = frames_read;
            ended_ = frames_read_ >= frame_limit_;
        }
        history_ = std::move(history);
        history_index_ = 0;
        history_bytes_ = history_.empty() ? 0 : history_.front().pixel_bytes();
        stage_index_ = 0;
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

bool ViewerSession::restart()
{
    return select_source(source_entry_);
}

void ViewerSession::select_cached_frame(std::size_t index)
{
    history_index_ = index;
    stage_index_ = std::min(stage_index_, result()->snapshots.size() - 1);
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
        FrameResult result = pipeline_.process(*raw, true);
        history_.push_back({std::move(*raw), std::move(result), frames_read_});
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

void ViewerSession::select_stage(std::size_t index)
{
    if (result() && index < result()->snapshots.size())
    {
        playing_ = false;
        stage_index_ = index;
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
const std::string& ViewerSession::pipeline_id() const noexcept
{
    return pipeline_id_;
}
const FrameResult* ViewerSession::result() const noexcept
{
    return history_.empty() ? nullptr : &history_[history_index_].processed;
}
const StageSnapshot* ViewerSession::snapshot() const noexcept
{
    return result() ? &result()->snapshots[stage_index_] : nullptr;
}
std::size_t ViewerSession::stage_index() const noexcept
{
    return stage_index_;
}
std::uint64_t ViewerSession::revision() const noexcept
{
    return revision_;
}

} // namespace visionlab::app
