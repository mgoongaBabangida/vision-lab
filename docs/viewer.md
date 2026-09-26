# Viewer workbench

The viewer executable uses SDL2 for its window/events, OpenGL 3.3 for textures and
rendering, and Dear ImGui for controls. `main.cpp` parses options and runs `ViewerApp`.
No renderer or UI types appear in the processing core or `ViewerSession`.

## Windows / Visual Studio 2022

Configure and build the `home-viewer` preset, open
`out/build/home-viewer/vision_lab.sln`, select Debug/x64, and press F5. The generated
solution selects `visionlab_viewer` as its startup project. The source folder defaults
to the checkout's `data/` directory even when launched from another working directory.
For Visual Studio Open Folder, use a local preset inheriting `windows-debug` and
select the viewer executable as the startup target.

OpenCV-enabled builds support image/video files. The locally configured home build
can use the official OpenCV 4.13 package extracted to `out/deps/opencv`; the ignored
user preset supplies its `OpenCV_DIR`. No DLLs, build products, or installed OpenCV
package files belong in Git. SDL2 and OpenCV DLLs are copied beside executables;
the existing shared dependencies are never modified by the project.

## Controls and state transitions

| Action | Result |
| --- | --- |
| Edit source folder + Refresh/Enter | Nonrecursive rescan; preserve the active source. |
| Select source | Open/decode first frame and construct a fresh pipeline, then commit the switch and pause. |
| Failed source/pipeline selection | Keep the previous source, frame, pipeline, and playback state; show the error. |
| Select pipeline | Create fresh stages, reprocess the displayed raw frame, clear old frame history, select Source, and pause. |
| Previous/Next stage or View selection | Select an existing snapshot and pause; do not process or decode anything. |
| Previous frame | Pause and select the previous cached frame, keeping the selected stage. |
| Next frame | Pause and select the next cached frame; at the newest frame, decode/process one new frame. |
| Play | Advance at approximately 30 inspection frames/s, keeping the selected stage. |
| Restart | Reopen current source and reset pipeline history at frame zero, paused. |
| End of input/frame limit | Keep last image and controls visible; Previous frame still works. Forward playback can replay the cache. |

An image source yields one frame. Its Play and both frame-navigation controls are disabled.
Switching pipelines remains useful for comparing that image. A new tracker begins
with no previous-frame history; use Restart to process the full sequence again.
Video capture still cannot reliably distinguish damaged-stream decode failure from
EOF after its first frame. Processing exceptions pause with a restart-required error.

Frame history has a 256 MiB pixel budget, including raw frames, processed images,
and stage snapshots. It always retains at least two frames when available, even if
those two exceed the budget. Older entries are evicted; Previous frame is disabled
at the oldest retained frame. Hover that button for an explanation. Restart returns
to frame zero and clears the cache, as does selecting a source or pipeline.

Browsing history does not rewind or rerun stateful stages. After returning to the
newest cached frame, processing continues from its existing state. Switching pipelines
while viewing an older frame reopens and decodes the source up to that position, then
resumes from its successor. Earlier frames are not processed by the new pipeline.
That repositioning can take time on long clips; a failure preserves the old session.

Space toggles playback, P/N select the previous/next frame, Left/Right browse stages, and Esc exits
when ImGui is not capturing the keyboard. Folder editing and focused widgets take
priority over global shortcuts.

## Sources and pipelines

`SourceCatalog` accepts PNG/JPEG/BMP/TIFF/WebP images and common MP4/AVI/MOV/MKV/M4V/
WebM/MPG/MPEG/WMV video extensions, case-insensitively. Extensions identify candidates,
not guaranteed codec support. Files are decoded only when selected. Synthetic input
is always present. Without OpenCV the catalog lists only synthetic input. Folder
errors preserve the last successful list. Folder watching and recursive scans are
not implemented; use Refresh after adding files.

`PipelineCatalog` maps stable IDs and display labels to factories. Pass-through is
the only shipped pipeline: it has a Source snapshot and no algorithm stages.
Add lessons in `apps/common/catalogs.cpp`; see `lessons/README.md`. The CLI uses the
same catalog with `--pipeline <id>`, so GUI selections do not define separate algorithms.

## Snapshots and rendering

`Pipeline::process(frame, true)` captures Source, then one image/overlay snapshot
after each stage. The source image and subsequent snapshots own their pixels.
`process(frame)` defaults to no snapshots, for headless runs. `FrameResult::boxes`
holds image-space rectangles and labels; the viewer draws them without modifying
processing pixels. A stage that changes geometry must transform or replace its boxes.
Stage timing excludes the snapshot copy and rendering.

`ViewerSession` retains original frames and their processed results in a bounded cache.
The history cursor selects which frame is displayed while the decoder and pipeline
stay at the newest processed frame. `Graphics` uploads a selected image only when
the frame revision or selected stage changes, fits its aspect ratio, and scales its
overlays to the same displayed rectangle. Nearest sampling keeps pixels sharp.

## Validation and current limits

Session tests use multiple test-only stages/pipelines to verify switching, independent
snapshots, overlays, preserved originals, EOF, eviction, and no extra processing while browsing.
OpenCV tests generate their own tiny media. `viewer_smoke` creates a hidden real
OpenGL window and renders three ImGui frames. Linux graphics CI uses Xvfb; SDL's
dummy video driver cannot provide this OpenGL context.

For layout inspection, run `visionlab_viewer --smoke-test --capture preview.bmp`.
This diagnostic exits after rendering and does not change normal EOF behavior.
It is not a replacement for testing interactive controls on a desktop.

Processing is synchronous for the initial lessons. Long decodes or expensive future
stages can delay UI responsiveness. Seeking, synchronized video playback, parameter
editors, async workers, and dockable panels are future additions, not hidden dependencies.
