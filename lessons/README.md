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
3. Register it in `apps/common/options.hpp`'s `make_pipeline()` (moving that function
   to a `.cpp` file is appropriate as it grows). Guard optional OpenCV composition
   consistently; the current OpenCV compile definition is private to `visionlab_app_common`.
4. Keep `cv::imshow`, event handling, and SDL/OpenGL out of the stage.
5. Add named, owned BGR8 debug images to `result.debug_images` and inspect with Tab.
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
        // Implement one operation here; preserve a snapshot if useful.
        result.debug_images.push_back({"before", result.frame.image});
    }
};
```

Registering the same stage at the shared composition point makes CLI and viewer
results comparable. Use the CLI for repeatable batch runs and the viewer for
inspection. Add controls/parameters only when the lesson has something to vary.
