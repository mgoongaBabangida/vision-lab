#include "viewer_app.hpp"
#include <exception>
#include <iostream>

int main(int argc, char** argv)
{
    try
    {
        visionlab::app::ViewerApp application(visionlab::app::parse_viewer_options(argc, argv));
        return application.run();
    }
    catch (const std::exception& error)
    {
        std::cerr << "visionlab_viewer: " << error.what() << '\n';
        return 1;
    }
}
