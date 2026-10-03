# Practice 09 - HSV color selection

Original Lesson 2, section 14. HSV separates hue (color family), saturation
(color vividness), and value (brightness). Range tests select pixels of a chosen
color. This can isolate a deliberately colored marker without training a model.
It is segmentation, not yet object detection: there are no boxes or object IDs.

Pipeline: Source -> HSV orange mask. Preserve the color input; grayscale would
discard the hue information we need. The stage converts BGR to HSV internally,
uses `cv::inRange`, then converts the binary mask to BGR8 for the viewer.
Raw HSV values must not be displayed as though they were BGR colors.

## Bounds

Standard 8-bit `COLOR_BGR2HSV` uses H=0..179 and S/V=0..255.
The example selects H=5..25, S=100..255, V=80..255. All three tests must pass;
the lower and upper endpoints are included. This corrects the original lesson's
strict-inequality shorthand for `inRange`.

The bounds are a starting point for orange, not a universal object detector.
Lighting, shadows, camera white balance, and desaturated reflections still matter.
Red straddles the ends of the hue scale and commonly needs two ranges combined
with OR; orange keeps this first example to one range.

## Observe

Run `python scripts/create_hsv_sample.py` from the repository root to generate
`data/hsv-color-sample.bmp` using only the Python standard library. The file is
local and Git-ignored; the script is portable and refuses to overwrite a file.

The six circles, from left to right:

| Row | Left | Middle | Right |
| --- | --- | --- | --- |
| Top | Bright orange | Dim orange | Pale orange |
| Bottom | White | Green | Blue |

In Compare mode, select Pass-through on the left and **Practice 09 - HSV orange mask**
on the right. Use the color sample as the shared source, then select the right
pane's final stage. Only the bright and dim orange circles should be white.
The pale circle fails the saturation cutoff; white fails it too.

First experiment: lower the S minimum from 100 to 20 in `hsv_color_stage.cpp`.
Predict whether the pale orange circle will appear. Restore 100 afterward;
the known-color regression test expects the baseline.

The mostly gray ball video is less useful for this example. The original synthetic
source also works and exposes a changing selected region during playback.

[OpenCV HSV range tutorial](https://docs.opencv.org/4.13.0/da/d97/tutorial_threshold_inRange.html).
CLI ID: `practice-09-hsv-color`.
