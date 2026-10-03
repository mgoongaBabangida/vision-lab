# Practice 03 - Gaussian blur

Original Lesson 2, section 2. Implement only `GaussianBlurStage::process` in
`src/opencv/gaussian_blur_stage.cpp`. The prepared pipeline is:

```text
Source -> Grayscale -> Gaussian blur (exercise)
```

It starts from the same grayscale input as Practice 02. This lets us compare the
filters without applying one blur on top of the other. The Gaussian stage is
unfinished: its output currently matches the Grayscale view.

## Two parameters

Mean blur weights every position equally. Gaussian blur emphasizes nearby pixels.
Its kernel size controls the finite neighborhood; sigma controls how broadly the
weights are spread within it. Increasing sigma at a fixed size makes the weights
less concentrated near the center. The finite kernel limits that spread.

Use the original lesson's values: **5x5** and **sigmaX = 1.5**. Omitting sigmaY
uses the same value vertically. The explicitly chosen kernel dimensions must be
positive and odd. Sigma zero requests automatic selection, not zero blur.
See the [GaussianBlur reference](https://docs.opencv.org/4.13.0/d4/d86/group__imgproc__filter.html).

The earlier 3x3 matrix with weights `1,2,1; 2,4,2; 1,2,1` divided by 16 illustrates
center weighting. It is not the actual 5x5 kernel for this exercise.

## Your implementation

Reuse the structure you wrote for mean blur:

1. Make a temporary BGR8 `cv::Mat` view of the image.
2. Declare an empty output matrix. Call `cv::GaussianBlur` with the view, output,
   `cv::Size(5, 5)`, and `1.5` as its first four arguments. Leave the remaining
   arguments at their defaults.
3. Use the output matrix's `copyTo` to update the original view.

Keep the frame metadata and image dimensions unchanged. No extra grayscale
conversion or pixel loop is needed; the pipeline already includes Grayscale.

## Observe

Build Debug/x64, press F5, and select **Practice 03 - Gaussian blur (exercise)**.
Pause on a detailed image and compare Grayscale with Gaussian blur. Switching
between Practice 02 and Practice 03 retains the raw current frame; select each
pipeline's final stage after switching.

Mean blur currently uses 3x3 while this exercise uses 5x5, so that comparison changes
both weighting and neighborhood size. To isolate weighting later, temporarily
change mean blur to 5x5 too, then restore its 3x3 baseline. Its known-pixel test
expects 3x3 and will fail during that experiment.

First get Gaussian blur working at 5x5 / 1.5. After review we will vary sigma while
keeping kernel size fixed. Gaussian smoothing still loses detail; it does not
identify which small features are noise and which are real objects.

The existing tests verify earlier lessons and pipeline infrastructure. Gaussian
pixel checks will follow your implementation.

CLI entry: `visionlab_cli --pipeline practice-03-gaussian-blur --frames 1` (OpenCV build).
