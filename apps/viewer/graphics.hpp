#pragma once

#include "viewer_session.hpp"
#include <SDL.h>
#include <SDL_opengl.h>
#include <filesystem>
#include <limits>

namespace visionlab::app
{

// SDL, OpenGL, and ImGui lifetime is confined to the viewer.
class Graphics
{
public:
    Graphics() = default;
    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;
    ~Graphics();
    void initialize(bool hidden);
    SDL_Window* window() const noexcept;
    void draw_image(const ViewerSession& session, std::size_t side = 0);
    void present(const std::filesystem::path& capture_path = {});

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext context_ = nullptr;
    bool sdl_ = false;
    bool imgui_ = false;
    bool platform_ = false;
    bool renderer_ = false;
    struct Texture
    {
        GLuint id = 0;
        std::uint64_t revision = std::numeric_limits<std::uint64_t>::max();
        std::size_t stage = std::numeric_limits<std::size_t>::max();
    };
    std::array<Texture, 2> textures_;
};

} // namespace visionlab::app
