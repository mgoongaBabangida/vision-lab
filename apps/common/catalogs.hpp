#pragma once

#include "visionlab/pipeline.hpp"
#include "visionlab/source.hpp"
#include <filesystem>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace visionlab::app
{

enum class SourceKind
{
    Synthetic,
    Image,
    Video
};

struct SourceEntry
{
    std::string label;
    std::filesystem::path path;
    SourceKind kind = SourceKind::Synthetic;
};

class SourceCatalog
{
public:
    // Nonrecursive scan. Extensions identify candidates; decoding validates them on selection.
    static std::vector<SourceEntry> scan(const std::filesystem::path& folder);
    static SourceEntry from_path(const std::filesystem::path& path);
    static bool media_available() noexcept;
};

std::unique_ptr<FrameSource> open_source(const SourceEntry& entry, std::uint64_t frame_limit);

struct PipelineDefinition
{
    std::string id;
    std::string label;
    std::function<Pipeline()> create;
};

class PipelineCatalog
{
public:
    PipelineCatalog();
    void add(PipelineDefinition definition);
    Pipeline create(const std::string& id) const;
    const std::vector<PipelineDefinition>& entries() const noexcept;

private:
    std::vector<PipelineDefinition> entries_;
};

} // namespace visionlab::app
