#include "catalogs.hpp"
#ifdef VISIONLAB_WITH_OPENCV
#include "visionlab/opencv/connected_components_stage.hpp"
#include "visionlab/opencv/canny_walkthrough.hpp"
#include "visionlab/opencv/sobel_smoothing_stage.hpp"
#include "visionlab/opencv/gradient_direction_stage.hpp"
#include "visionlab/opencv/morphology_ex_stage.hpp"
#include "visionlab/opencv/otsu_threshold_stage.hpp"
#include "visionlab/opencv/adaptive_threshold_stage.hpp"
#include "visionlab/opencv/hsv_color_stage.hpp"
#include "visionlab/opencv/erosion_stage.hpp"
#include "visionlab/opencv/dilation_stage.hpp"
#include "visionlab/opencv/binary_threshold_stage.hpp"
#include "visionlab/opencv/canny_stage.hpp"
#include "visionlab/opencv/gaussian_blur_stage.hpp"
#include "visionlab/opencv/grayscale_stage.hpp"
#include "visionlab/opencv/image_source.hpp"
#include "visionlab/opencv/mean_blur_stage.hpp"
#include "visionlab/opencv/sobel_x_stage.hpp"
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
#ifdef VISIONLAB_WITH_OPENCV
    add({"practice-01-grayscale", "Practice 01 - Grayscale", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             return pipeline;
         }});
    add({"practice-02-mean-blur", "Practice 02 - Mean blur (exercise)", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<MeanBlurStage>());
             return pipeline;
         }});
    add({"practice-03-gaussian-blur", "Practice 03 - Gaussian blur (exercise)", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             return pipeline;
         }});
    add({"practice-04-sobel-x", "Practice 04 - Sobel magnitude", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<SobelXStage>());
             return pipeline;
         }});
    add({"practice-05-canny", "Practice 05 - Canny (exercise)", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<CannyStage>());
             return pipeline;
         }});
    add({"practice-06-binary-threshold", "Practice 06 - Binary threshold", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             return pipeline;
         }});
    add({"practice-07-erosion", "Practice 07 - Erosion", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<ErosionStage>());
             return pipeline;
         }});
    add({"practice-07-dilation", "Practice 07 - Dilation", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<DilationStage>());
             return pipeline;
         }});
    add({"practice-08-opening", "Practice 08 - Opening", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<ErosionStage>());
             pipeline.add(std::make_unique<DilationStage>());
             return pipeline;
         }});
    add({"practice-08-closing", "Practice 08 - Closing", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<DilationStage>());
             pipeline.add(std::make_unique<ErosionStage>());
             return pipeline;
         }});
    add({"practice-09-hsv-color", "Practice 09 - HSV orange mask", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<HsvColorStage>());
             return pipeline;
         }});
    add({"practice-10-adaptive-threshold", "Practice 10 - Adaptive threshold", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<AdaptiveThresholdStage>());
             return pipeline;
         }});
    add({"practice-11-otsu-threshold", "Practice 11 - Otsu threshold", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<OtsuThresholdStage>());
             return pipeline;
         }});
    add({"practice-12-opening-ex", "Practice 12 - Opening (morphologyEx)", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<MorphologyExStage>(MorphologyOperation::Opening));
             return pipeline;
         }});
    add({"practice-12-closing-ex", "Practice 12 - Closing (morphologyEx)", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<MorphologyExStage>(MorphologyOperation::Closing));
             return pipeline;
         }});
    add({"practice-13-square", "Practice 13 - Square 5x5", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<DilationStage>(KernelShape::Rectangle, 5, 5));
             return pipeline;
         }});
    add({"practice-13-ellipse", "Practice 13 - Ellipse 5x5", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<DilationStage>(KernelShape::Ellipse, 5, 5));
             return pipeline;
         }});
    add({"practice-13-cross", "Practice 13 - Cross 5x5", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<DilationStage>(KernelShape::Cross, 5, 5));
             return pipeline;
         }});
    add({"practice-13-horizontal", "Practice 13 - Horizontal 9x1", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<DilationStage>(KernelShape::Rectangle, 9, 1));
             return pipeline;
         }});
    add({"practice-13-vertical", "Practice 13 - Vertical 1x9", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<BinaryThresholdStage>());
             pipeline.add(std::make_unique<DilationStage>(KernelShape::Rectangle, 1, 9));
             return pipeline;
         }});
    add({"practice-14-gradient-direction", "Practice 14 - Gradient direction", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             pipeline.add(std::make_unique<GradientDirectionStage>());
             return pipeline;
         }});
    add({"practice-15-x-difference", "Practice 15 - X without smoothing", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<SobelSmoothingStage>(false));
             return pipeline;
         }});
    add({"practice-15-sobel-smoothing", "Practice 15 - X with smoothing", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<SobelSmoothingStage>(true));
             return pipeline;
         }});
    add({"practice-16-canny-walkthrough", "Practice 16 - Inside Canny", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<GrayscaleStage>());
             pipeline.add(std::make_unique<GaussianBlurStage>());
             add_canny_walkthrough(pipeline);
             return pipeline;
         }});
    add({"practice-17-hsv-cleanup", "Practice 17 - HSV mask cleanup", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<HsvColorStage>());
             pipeline.add(std::make_unique<MorphologyExStage>(MorphologyOperation::Opening));
             pipeline.add(std::make_unique<MorphologyExStage>(MorphologyOperation::Closing));
             return pipeline;
         }});
    add({"practice-18-connected-components", "Practice 18 - Connected components", []
         {
             Pipeline pipeline;
             pipeline.add(std::make_unique<HsvColorStage>());
             pipeline.add(std::make_unique<MorphologyExStage>(MorphologyOperation::Opening));
             pipeline.add(std::make_unique<MorphologyExStage>(MorphologyOperation::Closing));
             pipeline.add(std::make_unique<ConnectedComponentsStage>());
             return pipeline;
         }});
#endif
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
