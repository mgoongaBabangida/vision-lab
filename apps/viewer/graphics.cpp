#include "graphics.hpp"
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_sdl.h>
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace visionlab::app
{
namespace
{

void capture_framebuffer(const std::filesystem::path& path, int width, int height)
{
    if (width <= 0 || height <= 0)
    {
        throw std::runtime_error("Cannot capture a zero-sized framebuffer");
    }
    const std::size_t row_bytes = static_cast<std::size_t>(width) * 3;
    std::vector<unsigned char> pixels(row_bytes * static_cast<std::size_t>(height));
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    for (int row = 0; row < height / 2; ++row)
    {
        unsigned char* top = pixels.data() + static_cast<std::size_t>(row) * row_bytes;
        unsigned char* bottom = pixels.data() + static_cast<std::size_t>(height - row - 1) * row_bytes;
        std::swap_ranges(top, top + row_bytes, bottom);
    }
    SDL_Surface* surface = SDL_CreateRGBSurfaceWithFormatFrom(pixels.data(), width, height, 24, width * 3, SDL_PIXELFORMAT_RGB24);
    if (!surface)
    {
        throw std::runtime_error(SDL_GetError());
    }
    const int status = SDL_SaveBMP(surface, path.u8string().c_str());
    SDL_FreeSurface(surface);
    if (status != 0)
    {
        throw std::runtime_error(SDL_GetError());
    }
}

} // namespace

Graphics::~Graphics()
{
    if (texture_)
    {
        glDeleteTextures(1, &texture_);
    }
    if (renderer_)
    {
        ImGui_ImplOpenGL3_Shutdown();
    }
    if (platform_)
    {
        ImGui_ImplSDL2_Shutdown();
    }
    if (imgui_)
    {
        ImGui::DestroyContext();
    }
    if (context_)
    {
        SDL_GL_DeleteContext(context_);
    }
    if (window_)
    {
        SDL_DestroyWindow(window_);
    }
    if (sdl_)
    {
        SDL_Quit();
    }
}

void Graphics::initialize(bool hidden)
{
    SDL_SetMainReady();
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        throw std::runtime_error(SDL_GetError());
    }
    sdl_ = true;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    const Uint32 flags =
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | (hidden ? SDL_WINDOW_HIDDEN : SDL_WINDOW_SHOWN);
    window_ = SDL_CreateWindow("Vision Lab", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1200, 850, flags);
    if (!window_)
    {
        throw std::runtime_error(SDL_GetError());
    }
    SDL_SetWindowMinimumSize(window_, 640, 480);
    context_ = SDL_GL_CreateContext(window_);
    if (!context_)
    {
        throw std::runtime_error(SDL_GetError());
    }
    if (SDL_GL_MakeCurrent(window_, context_) != 0)
    {
        throw std::runtime_error(SDL_GetError());
    }
    SDL_GL_SetSwapInterval(1);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imgui_ = true;
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().FramePadding = ImVec2(8, 6);
    ImGui::GetStyle().ItemSpacing = ImVec2(10, 9);
    platform_ = ImGui_ImplSDL2_InitForOpenGL(window_, context_);
    if (!platform_)
    {
        throw std::runtime_error("Cannot initialize ImGui's SDL backend");
    }
    renderer_ = ImGui_ImplOpenGL3_Init("#version 330 core");
    if (!renderer_)
    {
        throw std::runtime_error("Cannot initialize ImGui's OpenGL backend");
    }
}

SDL_Window* Graphics::window() const noexcept
{
    return window_;
}

void Graphics::draw_image(const ViewerSession& session)
{
    const StageSnapshot* snapshot = session.snapshot();
    if (!snapshot)
    {
        return;
    }
    if (uploaded_revision_ != session.revision() || uploaded_stage_ != session.stage_index())
    {
        const Image& image = snapshot->image;
        if (!texture_)
        {
            glGenTextures(1, &texture_);
        }
        glBindTexture(GL_TEXTURE_2D, texture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, image.width(), image.height(), 0, GL_BGR, GL_UNSIGNED_BYTE, image.data());
        glBindTexture(GL_TEXTURE_2D, 0);
        if (glGetError() != GL_NO_ERROR)
        {
            throw std::runtime_error("Cannot upload image texture");
        }
        uploaded_revision_ = session.revision();
        uploaded_stage_ = session.stage_index();
    }
    const ImVec2 available = ImGui::GetContentRegionAvail();
    const float scale =
        std::min(available.x / static_cast<float>(snapshot->image.width()), available.y / static_cast<float>(snapshot->image.height()));
    if (scale <= 0)
    {
        return;
    }
    const ImVec2 size(snapshot->image.width() * scale, snapshot->image.height() * scale);
    const ImVec2 cursor = ImGui::GetCursorPos();
    ImGui::SetCursorPos(ImVec2(cursor.x + (available.x - size.x) * 0.5f, cursor.y + (available.y - size.y) * 0.5f));
    const ImVec2 origin = ImGui::GetCursorScreenPos();
    ImGui::Image(reinterpret_cast<ImTextureID>(static_cast<intptr_t>(texture_)), size);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->PushClipRect(origin, ImVec2(origin.x + size.x, origin.y + size.y), true);
    for (const BoxOverlay& box : snapshot->boxes)
    {
        const ImVec2 minimum(origin.x + box.x * scale, origin.y + box.y * scale);
        const ImVec2 maximum(minimum.x + box.width * scale, minimum.y + box.height * scale);
        draw_list->AddRect(minimum, maximum, IM_COL32(100, 235, 155, 255), 0, 0, 2.0f);
        if (!box.label.empty())
        {
            draw_list->AddText(minimum, IM_COL32(100, 235, 155, 255), box.label.c_str());
        }
    }
    draw_list->PopClipRect();
}

void Graphics::present(const std::filesystem::path& capture_path)
{
    int width = 0;
    int height = 0;
    SDL_GL_GetDrawableSize(window_, &width, &height);
    glViewport(0, 0, width, height);
    glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    if (!capture_path.empty())
    {
        capture_framebuffer(capture_path, width, height);
    }
    SDL_GL_SwapWindow(window_);
}

} // namespace visionlab::app
