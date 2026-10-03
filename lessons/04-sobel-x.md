# Practice 04 - Sobel X

Continue Lesson 2 with brightness gradients. Implement only `SobelXStage::process`
in `src/opencv/sobel_x_stage.cpp`.

Pipeline: Source -> Grayscale -> Gaussian blur -> Sobel X.
The Sobel scaffold currently leaves its input unchanged.

## Your implementation

1. Create the same BGR8 image view used in earlier exercises.
2. Declare a separate gradient matrix. Call `cv::Sobel` with the image view,
   gradient matrix, `CV_32F`, `1`, `0`, and `3` as its first six arguments.
   These select floating-point depth, the first X derivative, no Y derivative,
   and a 3x3 kernel. Leave the remaining arguments at their defaults.
3. Declare a separate display matrix. Call `cv::convertScaleAbs` with the
   gradient as input and this display matrix as output, using the defaults.
4. Copy the display matrix back into the original image view with `copyTo`.

Our grayscale stage stores identical gray values in three BGR channels for the
viewer. Sobel processes each channel separately and preserves their count;
`CV_32F` selects depth, not channel count. The gradient therefore has three float
channels, and the display matrix has three 8-bit channels. No additional color
conversion is needed here. Do not copy the float gradient directly into the BGR8 view.

## Observe

Use the Gaussian baseline of 5x5 / sigma 1.5 for this exercise. Build and press F5,
then select **Practice 04 - Sobel X (exercise)**. Compare Gaussian blur with Sobel X.
Vertical boundaries should stand out; uniform regions should be dark.

The float gradient keeps positive and negative changes. The display conversion
takes their absolute values and clips values above 255, so this picture loses
direction and can saturate. It is a visualization, not the original gradient.

Keep dx=1 and dy=0 for the first implementation; we will compare Y afterward.
CLI entry: `visionlab_cli --pipeline practice-04-sobel-x --frames 1`.
