#include "options.hpp"
#include <charconv>
#include <stdexcept>
#include <string_view>
#include <system_error>

namespace visionlab::app
{

Options parse_options(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; ++i)
    {
        const std::string_view argument(argv[i]);
        if (argument == "--help" || argument == "-h")
        {
            options.help = true;
        }
        else if (argument == "--frames")
        {
            if (++i == argc)
            {
                throw std::invalid_argument("--frames requires a positive integer");
            }
            const std::string_view value(argv[i]);
            const std::from_chars_result parsed = std::from_chars(value.data(), value.data() + value.size(), options.frames);
            if (parsed.ec != std::errc{} || parsed.ptr != value.data() + value.size() || options.frames == 0)
            {
                throw std::invalid_argument("--frames requires a positive integer");
            }
        }
        else if (argument == "--pipeline")
        {
            if (++i == argc || std::string_view(argv[i]).empty())
            {
                throw std::invalid_argument("--pipeline requires a pipeline ID");
            }
            options.pipeline = argv[i];
        }
        else if (argument == "--input")
        {
            if (++i == argc || std::string_view(argv[i]).empty())
            {
                throw std::invalid_argument("--input requires a video-file path");
            }
            options.input = argv[i];
        }
        else
        {
            throw std::invalid_argument("Unknown option: " + std::string(argument));
        }
    }
    return options;
}

std::string usage(const std::string& executable)
{
    return "Usage: " + executable +
           " [--frames N] [--input image-or-video] [--pipeline ID] [--help]\n"
           "Default: 300 synthetic frames. --frames sets the maximum frame count.\n"
           "File input requires VISIONLAB_WITH_OPENCV=ON. Default pipeline: pass-through.\n";
}

std::unique_ptr<FrameSource> make_source(const Options& options)
{
    if (options.input.empty())
    {
        return std::make_unique<SyntheticSource>(options.frames);
    }
    return open_source(SourceCatalog::from_path(std::filesystem::u8path(options.input)), options.frames);
}

} // namespace visionlab::app
