#include "catalogs.hpp"
#ifdef VISIONLAB_WITH_OPENCV
#include "visionlab/opencv/image_source.hpp"
#include "visionlab/opencv/video_source.hpp"
#endif
#include <algorithm>
#include <cctype>
#include <stdexcept>
#include <utility>

namespace visionlab::app
{
namespace
{

std::string extension_of(const std::filesystem::path& path)
{
    std::string extension = path.extension().u8string();
    for (char& character : extension)
    {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return extension;
}

bool is_image_extension(const std::string& extension)
{
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".bmp" || extension == ".tif" ||
           extension == ".tiff" || extension == ".webp";
}

bool is_video_extension(const std::string& extension)
{
    return extension == ".mp4" || extension == ".avi" || extension == ".mov" || extension == ".mkv" || extension == ".m4v" ||
           extension == ".webm" || extension == ".mpg" || extension == ".mpeg" || extension == ".wmv";
}

} // namespace

bool SourceCatalog::media_available() noexcept
{
#ifdef VISIONLAB_WITH_OPENCV
    return true;
#else
    return false;
#endif
}

SourceEntry SourceCatalog::from_path(const std::filesystem::path& path)
{
    const std::string extension = extension_of(path);
    if (!is_image_extension(extension) && !is_video_extension(extension))
    {
        throw std::invalid_argument("Unsupported source extension: " + extension);
    }
    return {path.filename().u8string(), std::filesystem::absolute(path),
            is_image_extension(extension) ? SourceKind::Image : SourceKind::Video};
}

std::vector<SourceEntry> SourceCatalog::scan(const std::filesystem::path& folder)
{
    std::vector<SourceEntry> result{{"Synthetic pattern", {}, SourceKind::Synthetic}};
    if (!std::filesystem::is_directory(folder))
    {
        throw std::runtime_error("Source folder does not exist: " + folder.u8string());
    }
    if (!media_available())
    {
        return result;
    }
    for (const std::filesystem::directory_entry& entry : std::filesystem::directory_iterator(folder))
    {
        const std::string extension = extension_of(entry.path());
        if (entry.is_regular_file() && (is_image_extension(extension) || is_video_extension(extension)))
        {
            result.push_back(from_path(entry.path()));
        }
    }
    std::sort(result.begin() + 1, result.end(),
              [](const SourceEntry& left, const SourceEntry& right)
              {
                  return left.label < right.label;
              });
    return result;
}

std::unique_ptr<FrameSource> open_source(const SourceEntry& entry, std::uint64_t frame_limit)
{
    if (entry.kind == SourceKind::Synthetic)
    {
        return std::make_unique<SyntheticSource>(frame_limit);
    }
#ifdef VISIONLAB_WITH_OPENCV
    if (entry.kind == SourceKind::Image)
    {
        return make_image_source(entry.path);
    }
    return make_video_source(entry.path.u8string());
#else
    throw std::runtime_error("File input requires VISIONLAB_WITH_OPENCV=ON.");
#endif
}

PipelineCatalog::PipelineCatalog()
{
    // Register lesson pipelines here. The CLI and viewer share this catalog.
    add({"pass-through", "Pass-through", []
         {
             return Pipeline{};
         }});
}

void PipelineCatalog::add(PipelineDefinition definition)
{
    if (definition.id.empty() || definition.label.empty() || !definition.create)
    {
        throw std::invalid_argument("Pipeline definitions need an ID, label, and factory");
    }
    for (const PipelineDefinition& existing : entries_)
    {
        if (existing.id == definition.id)
        {
            throw std::invalid_argument("Duplicate pipeline ID: " + definition.id);
        }
    }
    entries_.push_back(std::move(definition));
}

Pipeline PipelineCatalog::create(const std::string& id) const
{
    for (const PipelineDefinition& definition : entries_)
    {
        if (definition.id == id)
        {
            return definition.create();
        }
    }
    throw std::invalid_argument("Unknown pipeline: " + id);
}

const std::vector<PipelineDefinition>& PipelineCatalog::entries() const noexcept
{
    return entries_;
}

} // namespace visionlab::app
