# Practice 16 - Inside Canny

Canny combines smoothing, gradients, thinning and connectivity to produce a binary
edge image. Select **Practice 16 - Inside Canny** and step through all eight views:

1. Source.
2. Grayscale.
3. Gaussian blur: the existing stage reduces noise before differentiation.
4. Gradient magnitude: signed Sobel X/Y give magnitude and angle. Display brightness
   clips at 255, but later calculations retain the original float values.
5. Non-maximum suppression: compare each magnitude with two neighbors along the
   gradient, across the edge. Keep a local peak and suppress the rest to zero.
   This thins broad responses; it is not a binary threshold or morphological erosion.
6. Threshold classification: magnitude >=200 is strong (white), 100..200 exclusive
   of 200 is weak (orange), and below 100 is discarded (black). These are gradient
   strengths, not source-pixel intensities. Only non-suppressed pixels qualify.
7. Hysteresis: start from every strong pixel, follow eight-connected weak/strong
   candidates recursively, and retain every reached pixel. Entire weak chains can
   survive; disconnected weak components disappear. No new pixels bridge gaps.
8. OpenCV Canny reference: `cv::Canny(savedGray, edges, 100, 200, 3, true)`.
   The saved input is the blurred grayscale image, not the preceding visualization.

The last two stages are binary: white means retained, including formerly weak
pixels. Hysteresis does not increase their measured gradient strength.

## Controlled example

Run `python scripts/create_canny_sample.py` if needed, refresh sources, and select
`canny-hysteresis-sample.bmp`. The left object fades from strong contrast above to
weak contrast below. The separate right object has only weak contrast.

Use Compare mode with this same pipeline on both sides:

- Compare Gradient magnitude with Non-maximum suppression to inspect thinning.
- Compare Thresholds with Hysteresis: orange on the lower left survives because it
  connects to white above; the separate orange rectangle disappears.
- Compare Hysteresis with OpenCV Canny reference, then try real video.

The fixture is tuned for the current shared 5x5 Gaussian sigma=3.5 and thresholds
100/200. Changing blur or thresholds can change which edges qualify as strong.

## Teaching implementation versus OpenCV

OpenCV does not expose these internal images through `cv::Canny`. The intermediate
steps here are an explicit teaching implementation, not extracted internal buffers.
It quantizes angles into four directions, breaks ties with `center > before` and
`center >= after`, clears the outermost border, and uses inclusive thresholds.
OpenCV has implementation-specific border, comparison and numerical choices, so
pixel-for-pixel equality is not promised. The last stage is the actual library call.

The factory creates private shared working data for its five stages, with fresh data
for each pipeline. Float analysis matrices are separate from BGR8 display snapshots.
Every frame overwrites the working data; frame navigation uses existing owned snapshots.
There are no visualization dependencies in the processing code.

Implementation: `src/opencv/canny_walkthrough.cpp`.
Pipeline ID: `practice-16-canny-walkthrough`.
Reference: [OpenCV Canny tutorial](https://docs.opencv.org/4.13.0/da/d22/tutorial_py_canny.html).

Next: HSV mask -> opening -> closing, then Lesson 3 objects and contours.
