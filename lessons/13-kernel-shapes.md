# Practice 13 - Kernel shapes

Original Lesson 2, section 24. A structuring element selects the neighboring
positions involved in morphology. Its nonzero entries participate; zero entries
are ignored. These are not Gaussian-style averaging weights.

We use dilation to expose the geometry: one isolated white pixel expands into
the centered kernel's footprint. All five variants use one iteration.

| Practice 13 variant | Kernel | What to observe |
| --- | --- | --- |
| Square | Rectangle 5x5 | Full square footprint |
| Ellipse | Ellipse 5x5 | Rounded, pixelated footprint with missing corners |
| Cross | Cross 5x5 | Horizontal/vertical arms, no off-axis pixels |
| Horizontal | Rectangle 9x1 | Growth along rows |
| Vertical | Rectangle 1x9 | Growth along columns |

Only the first three have the same bounding dimensions; the last two intentionally
change aspect ratio. OpenCV's 3x3 ellipse and cross coincide, so we use 5x5 to show
their difference. Ellipse means a raster approximation, not a perfect circle.

## Compare

Refresh and select `kernel-shapes-sample.bmp`. The top row has three isolated
white pixels. Below are a broken horizontal line and a broken vertical line.
The small 160x100 sample is enlarged with nearest-neighbor sampling in the viewer.
Reproduce it with `python scripts/create_kernel_sample.py`.

Select two Practice 13 pipelines and their final Dilation stages in Compare mode:
1. Square versus Ellipse, then Ellipse versus Cross: inspect the top pixels.
2. Horizontal versus Vertical: inspect which broken line becomes connected.

These pipelines use Source -> Binary threshold -> Dilation, with no blur.
Thresholding guarantees a binary mask; avoiding blur preserves the sample geometry.
The shape and size are supplied to DilationStage in `apps/common/catalogs.cpp`.
Earlier exercises retain their original 3x3 square default.

Kernel choice expresses a geometric assumption. A horizontal kernel can bridge
horizontal gaps while leaving vertical gaps open; it can also join unrelated objects.

[OpenCV kernel examples](https://docs.opencv.org/4.13.0/d9/d61/tutorial_py_morphological_ops.html).
