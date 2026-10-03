# Practice 07 - Erosion and dilation

Both pipelines start with Source -> Grayscale -> Gaussian blur -> Binary threshold.
Select **Practice 07 - Erosion** or **Practice 07 - Dilation** in the viewer.
Each applies just its own operation to the threshold mask.

Implementation files: `src/opencv/erosion_stage.cpp` and `src/opencv/dilation_stage.cpp`.
Each creates a 3x3 square with `cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3))`,
then calls `cv::erode` or `cv::dilate` once. A separate output is copied into the
original BGR8 image, preserving the frame's dimensions and metadata.

The binary mask is stored as three identical channels for display. Morphology
processes them independently, producing three identical result channels.
The calls use a centered kernel, one iteration, and OpenCV's default morphology
border handling (neutral outside-image values for the min/max operation).

- Erosion shrinks white regions. A 3x3 white square surrounded by black becomes
  one white pixel; narrower features can vanish.
- Dilation expands white regions. The same square becomes a 5x5 white square.

Pause the ball clip. Compare Binary threshold with the final stage using
Previous/Next stage. Then switch pipelines and select the final stage again.
Watch small reflections shrink or disappear with erosion and grow with dilation.
Neither operation distinguishes useful features from noise.

Keep threshold and blur settings fixed for this comparison. Later try a 5x5
kernel, then restore 3x3; the known-mask regression check expects the 3x3 baseline.
Opening and closing will combine these operations in the next exercise.
