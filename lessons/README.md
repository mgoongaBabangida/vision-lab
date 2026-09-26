# Incremental lessons

The initial pipeline contains no stages. Keep each new experiment understandable
and runnable with both frontends before moving on.

| Step | Practice | Useful debug output |
| --- | --- | --- |
| 0 | Build, breakpoints, frame ownership, pause/step | Synthetic image and frame metadata |
| 1 | Pixels, BGR/RGB, grayscale, ROI, resize | Original, converted image, crop |
| 2 | Blur, threshold, morphology, edges, contours | One named image after each operation |
| 3 | Features, descriptors, matching, robust fitting | Keypoints, matches, inlier overlays |
| 4 | Frame differences, optical flow, camera motion | Previous/current frames and flow |
| 5 | Intrinsics, calibration, distortion, projection | Reprojection and undistortion views |
| Later | Detector, tracker, evaluation, ONNX inference | Boxes, IDs, trajectories, metrics |

These are a practice sequence, not completed implementations.

## Add the first real stage

1. Enable OpenCV and add the stage under `src/opencv/` with a public header under
   `include/visionlab/opencv/`. Add the source to `visionlab_opencv` in CMake.
2. Implement `Stage::name()` and `Stage::process(FrameResult&)`.
3. Register a named factory in `PipelineCatalog::PipelineCatalog()` in
   `apps/common/catalogs.cpp`, adding your stages in order. Guard OpenCV-dependent
   entries with `#ifdef VISIONLAB_WITH_OPENCV`. The dropdown and CLI share this catalog.
4. Keep `cv::imshow`, event handling, and SDL/OpenGL out of the stage.
5. Write the stage's output to `result.frame.image`, and optional image-space boxes
   to `result.boxes`. The pipeline automatically saves an owned image/overlay snapshot
   after every stage when inspected in the viewer. Browse with Previous/Next stage.
6. Validate the lesson's actual behavior on a tiny known input, then try video.

A stage has this shape (this illustrative code is not compiled yet):

```cpp
class FirstLesson final : public visionlab::Stage
{
public:
    std::string_view name() const noexcept override
    {
        return "first-lesson";
    }
    void process(visionlab::FrameResult& result) override
    {
        // Implement one operation on result.frame.image here.
        // The pipeline captures its output automatically for the viewer.
    }
};
```

Registering the same stage at the shared composition point makes CLI and viewer
results comparable. Use the CLI for repeatable batch runs and the viewer for
inspection. Add controls/parameters only when the lesson has something to vary.

The factory has this shape once `FirstLesson` is implemented:

```cpp
add({"first-lesson", "First lesson", []
{
    Pipeline pipeline;
    pipeline.add(std::make_unique<FirstLesson>());
    return pipeline;
}});
```

The ID is accepted by `visionlab_cli --pipeline first-lesson`; the label appears
in the viewer. Each factory call must produce fresh stage instances, so source and
pipeline changes cannot inherit previous tracking history. Split a multi-step lesson
into stages when you want to inspect each intermediate image. Retain the source's
original frame index/time when modifying its pixels. Box coordinates must refer to
the current stage image, especially after resizing or cropping.
