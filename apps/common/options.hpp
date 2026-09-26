#pragma once

#include "visionlab/pipeline.hpp"
#include "visionlab/source.hpp"
#include "catalogs.hpp"
#include <memory>
#include <string>

namespace visionlab::app
{

struct Options
{
    std::uint64_t frames = 300;
    std::string input;
    bool help = false;
    std::string pipeline = "pass-through";
};

Options parse_options(int argc, char** argv);
std::string usage(const std::string& executable);
std::unique_ptr<FrameSource> make_source(const Options& options);

inline Pipeline make_pipeline(const std::string& id = "pass-through")
{
    return PipelineCatalog{}.create(id);
}

} // namespace visionlab::app
