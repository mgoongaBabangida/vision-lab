# Initial decisions

## CMake 3.23+ and C++17

CMake provides one target graph for MSVC, GCC, and Clang. Presets keep the Windows
and WSL configuration reproducible, and CTest supplies the same test entry point
locally and in CI. Visual Studio and VS Code can both consume CMake projects.
C++17 provides the small standard-library features this scaffold needs without
making newer language/toolchain features part of the learning task.

Premake remains appropriate for the existing engine. This independent vision
project benefits from the CMake package ecosystem used by OpenCV and SDL2; sharing
libraries does not require sharing the engine's generator or project structure.

References: [CMake presets](https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html),
[Visual Studio presets](https://learn.microsoft.com/en-us/cpp/build/cmake-presets-vs?view=msvc-170),
[OpenCV with CMake](https://docs.opencv.org/4.x/db/df5/tutorial_linux_gcc_cmake.html).

## Dependencies are explicit and optional

Start with no third-party libraries so a clean headless build works immediately.
Use `find_package` for OpenCV and installed SDL2 packages. For the existing Windows
layout, a small imported SDL2 target reads the shared headers/import library and
copies its runtime DLL to the build directory. Absolute machine paths belong only
in ignored user presets. Never search the engine source tree as a build dependency.

Windows binary packages cannot be linked into Linux executables. WSL uses Linux
packages or libraries compiled inside WSL. The project does not fetch dependencies
silently, vendor the engine, or commit binary libraries and datasets.

## Minimal scaffold, then one concept at a time

Only synthetic input, optional file decoding, a pass-through pipeline, and two
frontends exist initially. The viewer provides folder/source selection, a named
pipeline catalog, pause/step controls, and source/stage snapshots. There are no implemented CV lessons, detector,
tracker, neural network, calibration suite, or flight-control integration.

Meaningful tests protect ownership, stage order, timing metadata, EOF, CLI errors,
video decoding, and the ability to build/run without a display. CI does not pretend
to test interactive controls or all camera/codec/platform combinations.

## Viewer UI

The viewer uses SDL2 + OpenGL 3.3 + Dear ImGui. A pinned copy of the engine's matched
ImGui core/backends lives in `external/imgui` with its license and provenance. This
keeps the build independent of the engine checkout and avoids network downloads at
configuration time. CMake discovers no graphics dependencies in a headless build.
The synchronous session is independently testable; add a worker when processing
becomes expensive enough to justify asynchronous state management.
