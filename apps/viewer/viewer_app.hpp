#pragma once

#include "options.hpp"
#include <filesystem>

namespace visionlab::app
{

struct ViewerOptions
{
    Options processing;
    std::filesystem::path source_folder;
    bool smoke_test = false;
    std::filesystem::path capture_path;
};

ViewerOptions parse_viewer_options(int argc, char** argv);

class ViewerApp
{
public:
    explicit ViewerApp(ViewerOptions options);
    int run();

private:
    ViewerOptions options_;
};

} // namespace visionlab::app
