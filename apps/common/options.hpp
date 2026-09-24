#pragma once

#include "visionlab/pipeline.hpp"
#include "visionlab/source.hpp"
#include <memory>
#include <string>

namespace visionlab::app
{

struct Options
{
    std::uint64_t frames = 300;
    std::string input;
    bool help = false;
};

Options parse_options(int argc, char** argv);
std::string usage(const std::string& executable);
std::unique_ptr<FrameSource> make_source(const Options& options);

// This is the single composition point shared by both frontends.
// Add a lesson stage here when that lesson is implemented.
inline Pipeline make_pipeline()
{
    return Pipeline{};
}

} // namespace visionlab::app
