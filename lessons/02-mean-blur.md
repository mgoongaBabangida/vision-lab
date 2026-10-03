# Practice 02 - Mean blur

This follows original Lesson 2, section 1: filtering with a pixel's neighborhood.
Grayscale is complete. This exercise adds only a 3x3 mean filter; Gaussian blur
comes next, after we inspect and discuss this result.

The prepared pipeline is:

```text
Source -> Grayscale -> Mean blur (exercise)
```

Completed by the learner in `src/opencv/mean_blur_stage.cpp`. The instructions
below remain as a reference. Known-pixel checks cover the 3x3 average, independent
channels, frame metadata, and preservation of the original source snapshot.

## One operation

A 3x3 mean filter averages nine neighboring values, including the center, using
equal weights of 1/9. For this interior neighborhood:

```text
10   10   10
10  100   10
10   10   10
```

the center becomes `(8 * 10 + 100) / 9 = 20`. The operation repeats across the image.
An area filled with 10 stays 10; an isolated bright point spreads into its surroundings.
This can suppress noise but also soften real edges and small objects.
OpenCV calls this a normalized box filter. See its
[smoothing tutorial](https://docs.opencv.org/4.13.0/d4/d13/tutorial_py_filtering.html).

Grayscale mixed channels within each pixel. Blur mixes neighboring pixels within
each channel. Our grayscale stage stores three equal channels, so all three remain
equal after filtering. No additional color conversion is needed.

## Your implementation

1. Reuse the temporary BGR8 `cv::Mat` view pattern from Practice 01.
2. Declare a separate empty `cv::Mat` named `blurred`. Call `cv::blur` with the view
   as input, `blurred` as output, and `cv::Size(3, 3)` as the kernel size. Leave the
   anchor and border arguments at their defaults for now.
3. Copy the output pixels back into the original view with `blurred.copyTo(view)`.
   Use your own variable name for `view`.

The destination dimensions and type match, so `copyTo` reuses the existing image
buffer. Assigning `view = blurred` would instead replace the local Mat header and
leave our `Image` unchanged. Keep all views local to `process`.

The kernel size is the neighborhood size, not a new image size. Output dimensions
and type remain unchanged. OpenCV handles neighborhoods crossing an image edge
using a border rule; we will examine that separately. Reference:
[C++ blur API](https://docs.opencv.org/4.13.0/d4/d86/group__imgproc__filter.html).

## Inspect and experiment

Build Debug/x64, press F5, and select **Practice 02 - Mean blur (exercise)**.
Pause and compare **Grayscale** with **Mean blur**, using Previous/Next stage on
the same frame. A sharp color photograph with text or fine detail works well.
On the synthetic pattern, watch the narrow marker and sharp transitions; smooth
gradients may change very little with a 3x3 kernel.

After 3x3 works, predict what changing only the kernel to 7x7 will do. Rebuild and
compare, then restore 3x3. Does smoothing preserve every small object in the image?

The automated mean-blur check assumes a 3x3 kernel. It should fail while you
experiment with 7x7, then pass again when you restore 3x3.

CLI entry: `visionlab_cli --pipeline practice-02-mean-blur --frames 1` (OpenCV build).
