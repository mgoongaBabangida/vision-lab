# Vision Lab

A small C++17 computer-vision workbench for learning, debugging, and experiments.
The same processing pipeline runs in a console application or an optional SDL2 viewer.

The initial pipeline is deliberately empty. Synthetic frames, video-file input,
stage timing, and debug-image presentation provide the scaffolding; image-processing
lessons, detection, tracking, and ML are added incrementally.

```text
Synthetic source / optional OpenCV video source
                    |
                    v
          Frame -> Pipeline -> FrameResult
                    |              |
              ordered stages       +--> CLI: CSV metadata, no window
                                   +--> SDL2 viewer: frame / debug images

Later: OpenCV stages -> detector -> tracker
       PyTorch training -> ONNX model -> C++ inference stage
```

## Quick start: Windows 10 + Visual Studio 2022

Install Visual Studio's **Desktop development with C++** workload and CMake 3.23+.
From the repository folder in PowerShell:

```powershell
cmake --preset windows-vs
cmake --build --preset windows-vs-debug --parallel
ctest --preset windows-vs-debug
.\out\build\windows-vs\bin\Debug\visionlab_cli.exe --frames 3
```

Open `out/build/windows-vs/vision_lab.sln` in Visual Studio to debug the generated
solution. Alternatively, use **File > Open > Folder**, select `windows-debug`
(Ninja), and choose `visionlab_cli.exe` as the startup target. Visual Studio supplies
the MSVC environment; from a terminal, run Ninja presets in **Developer PowerShell
for VS 2022**. `windows-vs-release` is also available.

No third-party dependency is needed for this first build.

## Quick start: Windows 11 + VS Code + WSL

Use a separate Linux clone, preferably in the WSL filesystem:

```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build gdb git
mkdir -p ~/projects
cd ~/projects
git clone https://github.com/mgoongaBabangida/vision-lab.git
cd vision-lab
cmake --preset linux-debug
cmake --build --preset linux-debug --parallel
ctest --preset linux-debug
./out/build/linux-debug/bin/visionlab_cli --frames 3
code .
```

Install the VS Code **WSL** extension on Windows. In the WSL window, install the
recommended **C/C++** and **CMake Tools** extensions. Select `linux-debug` and use
CMake Tools' build/debug commands. `linux-release` is available too.

Linux needs its own libraries; Windows `.lib` and `.dll` files cannot be reused in WSL.
Keep separate clones/build directories for Windows and Linux and exchange code via Git.

## Enable the viewer at home

For this directory arrangement:

```text
<parent>/
  engine/
  vision-lab/
  third_party/SDL/include/SDL.h
  third_party/SDL/lib/x64/SDL2.lib
  third_party/SDL/lib/x64/SDL2.dll
```

```powershell
Copy-Item CMakeUserPresets.json.example CMakeUserPresets.json
cmake --preset home-viewer
cmake --build --preset home-viewer --parallel
ctest --preset home-viewer
.\out\build\home-viewer\bin\Debug\visionlab_viewer.exe --frames 900
```

`CMakeUserPresets.json` is ignored by Git. The shared folder is read only to this
build; SDL2.dll is copied beside the viewer. The engine is not linked or modified.
The example uses a generated VS solution; for Visual Studio Open Folder, change
its inherited preset from `windows-vs` to `windows-debug`.

The viewer offers **Space** to pause, **N** to advance one frame, **Tab** to cycle
the current frame and named debug images, and **Esc** to quit. It fits the image
without changing aspect ratio. Playback is approximately 30 inspection frames/s,
independent of the input FPS; it exits at EOF or the frame limit. It is not yet a
media player with seeking or synchronized playback.

On WSL/Linux, install SDL2 and enable the viewer in a separate build:

```bash
sudo apt-get install -y libsdl2-dev
cmake -S . -B out/build/linux-viewer -G Ninja -DCMAKE_BUILD_TYPE=Debug -DVISIONLAB_BUILD_VIEWER=ON
cmake --build out/build/linux-viewer --parallel
ctest --test-dir out/build/linux-viewer --output-on-failure
./out/build/linux-viewer/bin/visionlab_viewer --frames 900
```

Interactive display needs WSLg or another working Linux display server. The CLI
and automatic viewer smoke test do not require a display server.

## Enable OpenCV video input

OpenCV 4 is optional. Only `core`, `imgproc`, and `videoio` are requested; no
`imshow`, `waitKey`, or HighGUI code enters the processing layer.

WSL/Linux:

```bash
sudo apt-get install -y libopencv-dev
cmake -S . -B out/build/linux-opencv -G Ninja -DCMAKE_BUILD_TYPE=Debug -DVISIONLAB_WITH_OPENCV=ON
cmake --build out/build/linux-opencv --parallel
ctest --test-dir out/build/linux-opencv --output-on-failure
./out/build/linux-opencv/bin/visionlab_cli --input data/clip.mp4 --frames 100
```

Windows: install/build a matching **x64 MSVC OpenCV 4** package. Supply the directory
containing `OpenCVConfig.cmake` as `OpenCV_DIR`, preferably in a local preset:

```powershell
cmake --preset windows-vs -DVISIONLAB_WITH_OPENCV=ON -DOpenCV_DIR="C:/path/to/opencv/build"
cmake --build --preset windows-vs-debug --parallel
# Add the package's actual DLL directory, e.g. x64/vc16/bin, for this shell session.
$env:PATH = "C:\path\to\opencv\build\x64\vc16\bin;" + $env:PATH
.\out\build\windows-vs\bin\Debug\visionlab_cli.exe --input data/clip.mp4 --frames 100
```

OpenCV package layouts vary. Match architecture, compiler ABI, and Debug/Release
libraries, and ensure its runtime/codec DLLs are discoverable when launching from
the terminal or debugger. No OpenCV download or installation happens during CMake
configuration. Both optional switches can be enabled together for video viewing.

The adapter currently uses `frame_index / reported_fps` as a **nominal timestamp**.
Unknown FPS produces an empty CSV timestamp. Variable-frame-rate timing, seeking,
live cameras, and strict decode-error reporting remain future work. After the first
frame, OpenCV's read failure is treated as EOF because it does not reliably distinguish
EOF from damaged input. The frame limit defaults to 300; it is a maximum for file input.

## Where to work next

- [Architecture and ownership](docs/architecture.md)
- [Lesson roadmap and adding a stage](lessons/README.md)
- [Build and dependency choices](docs/decisions.md)
- [Experiment notes template](experiments/README.md)
- [Future training and model contract](training/README.md)

`visionlab_cli` writes CSV to stdout and a summary/errors to stderr. Redirect stdout
to a file under `runs/` when recording experiments. Local video, model weights,
build outputs, and machine-specific paths stay out of Git.

CI builds the headless project on Windows and Linux, then builds and tests the
OpenCV and SDL2 adapters on Linux. Viewer tests use SDL's dummy video driver.
