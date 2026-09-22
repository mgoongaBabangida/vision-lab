#include "options.hpp"
#include <SDL.h>
#include <chrono>
#include <exception>
#include <iostream>
#include <limits>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace {

void check(int status) {
    if (status < 0) { throw std::runtime_error(SDL_GetError()); }
}

struct SdlLifetime {
    SdlLifetime() {
        SDL_SetMainReady();
        check(SDL_Init(SDL_INIT_VIDEO));
    }
    ~SdlLifetime() { SDL_Quit(); }
};

template <typename T, void (*Destroy)(T*)>
using SdlPointer = std::unique_ptr<T, decltype(Destroy)>;

} // namespace

int main(int argc, char** argv) {
    try {
        const auto options = visionlab::app::parse_options(argc, argv);
        if (options.help) {
            std::cout << visionlab::app::usage("visionlab_viewer")
                      << "Space: pause/resume. N: next frame (pauses). Tab: cycle debug images. Esc: quit.\n";
            return 0;
        }
        auto source = visionlab::app::make_source(options);
        auto pipeline = visionlab::app::make_pipeline();
        const SdlLifetime sdl;
        SdlPointer<SDL_Window, SDL_DestroyWindow> window(
            SDL_CreateWindow("Vision Lab", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                             960, 720, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI), SDL_DestroyWindow);
        if (!window) { throw std::runtime_error(SDL_GetError()); }
        SdlPointer<SDL_Renderer, SDL_DestroyRenderer> renderer(
            SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_ACCELERATED), SDL_DestroyRenderer);
        if (!renderer) {
            renderer.reset(SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_SOFTWARE));
        }
        if (!renderer) { throw std::runtime_error(SDL_GetError()); }
        SdlPointer<SDL_Texture, SDL_DestroyTexture> texture(nullptr, SDL_DestroyTexture);
        int texture_width = 0;
        int texture_height = 0;
        bool running = true;
        bool paused = false;
        std::size_t selected_view = 0;
        std::uint64_t count = 0;
        std::optional<visionlab::FrameResult> latest;
        while (running) {
            const auto next_tick = std::chrono::steady_clock::now() + std::chrono::milliseconds(33);
            bool step = false;
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) { running = false; }
                if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
                    switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE: running = false; break;
                    case SDLK_SPACE: paused = !paused; break;
                    case SDLK_n: paused = true; step = true; break;
                    case SDLK_TAB: ++selected_view; break;
                    default: break;
                    }
                }
            }
            if (!running) { break; }
            if (!paused || step || !latest) {
                if (count >= options.frames) { break; }
                auto frame = source->next();
                if (!frame) { break; }
                latest = pipeline.process(std::move(*frame));
                ++count;
            }
            selected_view %= latest->debug_images.size() + 1;
            const auto& image = selected_view == 0 ? latest->frame.image
                                                  : latest->debug_images[selected_view - 1].image;
            const auto label = selected_view == 0 ? std::string("frame")
                                                  : latest->debug_images[selected_view - 1].name;
            if (image.stride_bytes() > static_cast<std::size_t>(std::numeric_limits<int>::max())) {
                throw std::runtime_error("Image row exceeds SDL's supported pitch");
            }
            if (!texture || image.width() != texture_width || image.height() != texture_height) {
                texture.reset(SDL_CreateTexture(renderer.get(), SDL_PIXELFORMAT_BGR24,
                    SDL_TEXTUREACCESS_STREAMING, image.width(), image.height()));
                if (!texture) { throw std::runtime_error(SDL_GetError()); }
                texture_width = image.width();
                texture_height = image.height();
                check(SDL_RenderSetLogicalSize(renderer.get(), texture_width, texture_height));
            }
            check(SDL_UpdateTexture(texture.get(), nullptr, image.data(), static_cast<int>(image.stride_bytes())));
            check(SDL_SetRenderDrawColor(renderer.get(), 16, 16, 16, 255));
            check(SDL_RenderClear(renderer.get()));
            check(SDL_RenderCopy(renderer.get(), texture.get(), nullptr, nullptr));
            SDL_RenderPresent(renderer.get());
            const auto title = "Vision Lab | " + label + " | frame " + std::to_string(latest->frame.index)
                + (paused ? " | PAUSED" : "") + " | Space: pause | N: step | Tab: view | Esc: quit";
            SDL_SetWindowTitle(window.get(), title.c_str());
            // Inspection playback only; media timestamps are independent of this UI pacing.
            std::this_thread::sleep_until(next_tick);
        }
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "visionlab_viewer: " << error.what() << '\n';
        return 1;
    }
}
