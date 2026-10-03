# Practice 14 - Gradient direction

Original Lesson 2, section 5: the gradient has magnitude and direction.
Magnitude describes how sharply brightness changes. Direction points toward
increasing brightness, perpendicular to the local edge:

`magnitude = sqrt(dx*dx + dy*dy)` and `angle = atan2(dy, dx)`.

Signed floating-point Sobel derivatives retain the difference between dark-to-light
and light-to-dark transitions. `cv::cartToPolar(..., true)` computes magnitude and
angle in degrees. We encode the angle as HSV hue, strength as value, then convert
to BGR for the viewer. These colors describe direction, not the source's colors.

| Increasing brightness toward | Angle | Display color |
| --- | --- | --- |
| Right | 0 degrees | Red |
| Down | 90 degrees | Yellow-green |
| Left | 180 degrees | Cyan |
| Up | 270 degrees | Violet |

Image y points down, so angles turn clockwise on screen. Flat regions are black;
their gradient direction is undefined. Value is `min(magnitude / 255, 1)` with a
fixed gain across frames. Strong responses clip, so brightness is not a measurement
of the full gradient magnitude. There is no per-frame min/max normalization.

## Try it

1. Generate the source with `python scripts/create_gradient_sample.py` if missing.
2. Refresh the source list and select `gradient-direction-sample.bmp`.
3. Enable Compare mode: Practice 04 - Sobel magnitude on the left,
   Practice 14 - Gradient direction on the right. Select each final stage.
4. Inspect opposite sides of the rectangle: the magnitude is similar, but the
   direction colors differ by half a turn. The circle traverses the hue wheel;
   the lower-contrast triangle has dimmer edges.
5. Try the synthetic video and real ball video. The colors show spatial brightness
   gradients, not motion direction.

Both pipelines use the current shared Gaussian settings. Direction matters for
Canny's non-maximum suppression: it tells Canny which neighboring magnitudes to
compare when thinning an edge. Sobel's smoothing and Canny thinning are next.

Implementation: `src/opencv/gradient_direction_stage.cpp`.
Pipeline ID: `practice-14-gradient-direction`.
References: [cartToPolar](https://docs.opencv.org/4.13.0/d2/de8/group__core__array.html)
and [HSV ranges](https://docs.opencv.org/4.13.0/de/d25/imgproc_color_conversions.html).
