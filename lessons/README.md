# Incremental lessons

Pass-through contains no stages. Each exercise introduces one small concept and
runs through the same pipeline in the CLI and viewer.

The learner writes the OpenCV operations. The assistant prepares necessary wiring,
explains the theory, gives hints, reviews the attempt, and helps test/debug it.
Do not fill in exercise TODOs for the learner unless they ask for the solution.

Completed by the learner: [Practice 01 - BGR to grayscale](01-grayscale.md) and
[Practice 02 - Mean blur](02-mean-blur.md), original Lesson 2, section 1.
[Practice 03 - Gaussian blur](03-gaussian-blur.md), original Lesson 2, section 2, is also completed by the learner.
[Practice 04 - Sobel X](04-sobel-x.md) is complete, including Y and combined gradient magnitude.
Its viewer label is now Sobel magnitude; the original CLI ID remains `practice-04-sobel-x`.
[Practice 05 - Canny](05-canny.md) is complete, including threshold experiments.
[Practice 06 - Binary threshold](06-binary-threshold.md) is implemented and explored.
[Practice 07 - Erosion and dilation](07-morphology.md) is implemented.
[Practice 08 - Opening and closing](08-opening-closing.md) is implemented and explored.
[Practice 09 - HSV color selection](09-hsv-color.md) is implemented and explored.
[Practice 10 - Adaptive threshold](10-adaptive-threshold.md) is implemented and explored.
[Practice 11 - Otsu threshold](11-otsu-threshold.md) is implemented and explored.
[Practice 12 - morphologyEx](12-morphology-ex.md) is implemented and explored.
[Practice 13 - Kernel shapes](13-kernel-shapes.md) is implemented and explored.
[Practice 14 - Gradient direction](14-gradient-direction.md) is implemented and explored.
[Practice 15 - Sobel smoothing](15-sobel-smoothing.md) is implemented and explored.
[Practice 16 - Inside Canny](16-canny-walkthrough.md) is implemented and explored.
[Practice 17 - HSV mask cleanup](17-hsv-cleanup.md) is implemented and explored.
Current exercise: [Practice 18 - Connected components](18-connected-components.md), starting Lesson 3.
Status: cleaned mask regions have colors, bounding boxes and pixel-area labels.

The planned Lesson 2 practical review is complete. Next: component area filtering, then contours.

For individual exercises, review the operation and build/run the affected target.
Use focused checks for changed behavior; reserve full suites and repeated platform checks
for shared infrastructure changes or unresolved concerns.

| Step | Practice | Useful debug output |
| --- | --- | --- |
| 0 | Build, breakpoints, frame ownership, pause/step | Synthetic image and frame metadata |
| 1 | Pixels, BGR/RGB, grayscale, ROI, resize | Original, converted image, crop |
| 2 | Mean/Gaussian blur, Sobel, Canny, thresholding, morphology | One named image after each operation |
| 3 | Connected components, contours, features, descriptors, matching | Boxes, centroids, keypoints, matches |
| 4 | Frame differences, optical flow, camera motion | Previous/current frames and flow |
| 5 | Intrinsics, calibration, distortion, projection | Reprojection and undistortion views |
| Later | Detector, tracker, evaluation, ONNX inference | Boxes, IDs, trajectories, metrics |

This follows the original OpenCV conversation's progression, with image fundamentals
before Lesson 2's filters, Lesson 3's objects/features, Lesson 4's motion, and Lesson 5's
geometry. Each row is split into small exercises; these are not completed implementations.

## Add another stage

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
