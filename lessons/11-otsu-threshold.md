# Practice 11 - Otsu threshold

Original Lesson 2, section 17. A brightness histogram counts pixels at each intensity.
Otsu chooses one cutoff that minimizes the weighted within-class variance of the
two resulting groups (equivalently, maximizes their between-class variance).
It works well when dark and bright pixel populations are reasonably separable.

Pipeline: Source -> Grayscale -> Gaussian blur -> Otsu threshold.
It uses the same preprocessing as our fixed and adaptive threshold pipelines.

The call in `src/opencv/otsu_threshold_stage.cpp` is:

```cpp
cv::threshold(gray, mask, 0.0, 255.0, cv::THRESH_BINARY | cv::THRESH_OTSU);
```

The supplied cutoff 0 is ignored because Otsu calculates it. The return value
contains that chosen cutoff; the output matrix contains the binary mask.
The operation needs a single-channel image, which we convert back to BGR for display.

## Observe

The generated `data/otsu-dim-sample.bmp` has three gray shapes on a darker background.
All source intensities are below 127, so our fixed threshold loses every shape.
Otsu separates the two intensity populations and reveals the shapes.
Reproduce it with `python scripts/create_otsu_sample.py`; the script refuses to
overwrite existing data and uses only Python's standard library.

Refresh, select the sample, then compare **Practice 06 - Binary threshold** on
the left with **Practice 11 - Otsu threshold** on the right. Select both final stages.
Inspect Source to see why a cutoff of 127 is unsuitable.

Then switch the shared source to `adaptive-lighting-sample.bmp` and compare Otsu
with **Practice 10 - Adaptive threshold**. Otsu still uses one global cutoff:
automatic selection does not solve spatially uneven illumination.

| Method | Where the cutoff comes from |
| --- | --- |
| Fixed | One number chosen by you |
| Adaptive | Local neighborhoods |
| Otsu | The whole frame's histogram |

On video, Otsu recalculates each frame, so changes in scene composition can make
the mask flicker. None of these methods recognizes object identity.

[OpenCV thresholding explanation](https://docs.opencv.org/4.13.0/d7/d4d/tutorial_py_thresholding.html).
CLI ID: `practice-11-otsu-threshold`.
