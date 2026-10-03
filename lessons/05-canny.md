# Practice 05 - Canny

Pipeline: Source -> Grayscale -> Gaussian blur -> Canny.
Implement only `CannyStage::process` in `src/opencv/canny_stage.cpp`.

Canny thins gradient responses and uses two thresholds: strong candidates seed
edges; weaker candidates survive when connected through candidates to a strong edge.
The result is a binary edge map, rather than the graded brightness of Sobel magnitude.

## Your implementation

1. Create the usual BGR8 image view.
2. Use `cv::cvtColor` with `cv::COLOR_BGR2GRAY` to create a single-channel matrix.
   The pipeline already removed color, but its viewer-compatible image still has
   three identical channels. This step changes the representation to one channel.
3. Call `cv::Canny` with gray input, a separate edges output, `50`, `150`, `3`, `true`.
   These are low/high thresholds, Sobel aperture size, and the L2 magnitude flag.
4. Use `cv::cvtColor` with `cv::COLOR_GRAY2BGR` to write edges into the original view.
   Do not copy the single-channel matrix directly into the three-channel image.

Feed Canny the blurred gray image, not your Sobel display. Canny computes its own
gradients; our separate Gaussian stage supplies smoothing. L2 uses the magnitude
formula from the previous exercise. Thresholds apply to gradients, not source brightness.

## Observe

Select **Practice 05 - Canny (exercise)** and the moving-ball clip. Until implemented,
the final stage matches Gaussian blur. Afterward expect white edges on black.
Pause and compare it with **Practice 04 - Sobel magnitude** on the same frame.

Keep the Gaussian settings fixed while learning the Canny thresholds. Your current
Gaussian stage uses sigma 3.5; choose 1.5 if returning to the earlier lesson baseline.
First get 50/150 working; then we will change thresholds together.

[OpenCV explanation](https://docs.opencv.org/4.13.0/da/d22/tutorial_py_canny.html).
CLI ID: `practice-05-canny`. Pixel checks follow your implementation.
