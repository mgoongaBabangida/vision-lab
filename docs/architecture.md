# Architecture

## Dependency direction

```text
visionlab_cli -------> visionlab_app_common -----> visionlab_core
visionlab_viewer ----> visionlab_app_common -----> visionlab_core
       |                       |
       v                       v (optional)
      SDL2                visionlab_opencv ------> visionlab_core
                               |
                               v
                             OpenCV
```

The core cannot include OpenCV, SDL, OpenGL, OS window APIs, or an application
entry point. The CLI never links SDL. Turning the viewer off removes its dependency
discovery and compilation entirely. OpenCV processing/video support can be enabled
with the viewer off. There is no requirement to create a window to process a frame.

## Small contracts

- `Image` owns a tightly packed BGR8 byte buffer; copying it is a deep copy.
- `Frame` owns an image, a zero-based source index, and optional media-relative seconds.
- `FrameSource::next()` returns an owned frame or `nullopt` at the end.
- `Stage` mutates a `FrameResult`; it may also add named debug-image snapshots.
- `Pipeline` calls stages in insertion order and records each processing duration.
- `FrameResult` crosses the processing/presentation boundary by value.

The synchronous app loop reads one frame, runs the pipeline, and consumes its result.
There is no worker thread, queue, global renderer, plugin registry, or async lifetime
problem yet. A pipeline is single-stream and not thread safe; future trackers can
hold previous-frame state inside a stage. Construct a new pipeline for a new video
until explicit reset semantics are needed.

The empty pipeline preserves the input frame. Stages can modify the working image;
they must save an owned debug snapshot before modification if the original should
remain visible. Publishing snapshots copies pixels deliberately. Profile before
introducing shared image ownership, GPU buffers, or zero-copy views.

OpenCV's adapter copies decoded pixels row by row, so the next decode cannot change
an earlier frame. A future OpenCV stage may wrap an `Image` in a temporary `cv::Mat`
view, but must never retain that view after the owning image is moved or destroyed.
Initially, convert grayscale/masks to BGR8 for display; introduce typed image formats
when a lesson needs to preserve float or single-channel data.

Pixel origin is top-left, x points right and y down. Media timestamps and processing
durations are separate measurements. OpenCV file timestamps are currently estimates
based on nominal FPS, not true container presentation timestamps. Do not use them
for calibrated velocity on variable-frame-rate footage.

## Error and UI ownership

Stages and sources throw on invalid configuration or processing errors. The app
prints a useful error and returns a nonzero exit code. GUI event handling, pacing,
pause/step controls, textures, and display selection belong only to the viewer.
The viewer receives completed results and does not implement image algorithms.

The first viewer uses SDL2's renderer, which can select an available backend.
OpenGL is not an explicit dependency yet. A later SDL/OpenGL/ImGui presentation
layer can replace it without changing the pipeline or headless app.

## Growth points

Add the smallest data structures demanded by each lesson. Detections can later be
plain image-space boxes/classes/scores; tracks can add stable IDs and trajectories.
The viewer should render these records as overlays rather than drawing into the
algorithm's working image. Avoid designing a generic detector/tracker framework
before an actual implementation establishes the contract.

PyTorch belongs in `training/` with a separate Python environment. Export ONNX
and a documented preprocessing/output contract. A C++ inference adapter can then
implement a stage, using OpenCV DNN or ONNX Runtime when those tradeoffs are known.
Training will not become a dependency of the C++ application.
