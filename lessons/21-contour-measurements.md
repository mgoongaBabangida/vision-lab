# Practice 21 - Contour measurements

This adds measurements to Practice 20's external contours:

`Source -> HSV mask -> Opening -> Closing -> Contour measurements`

For each contour we call:

```cpp
const double area = cv::contourArea(contour);
const double perimeter = cv::arcLength(contour, true);
const cv::Rect bounds = cv::boundingRect(contour);
```

- `contourArea`: geometric area enclosed by the polygon, in square pixels.
  The default returns an unsigned magnitude regardless of traversal orientation.
- `arcLength`: sum of Euclidean distances between successive points, in pixels.
  `true` closes the boundary by including the final-to-first segment.
- `boundingRect`: the enclosing axis-aligned pixel rectangle, with x, y, width and height.

The display labels use `A` for area and `P` for perimeter, rounded to one decimal
place. Measurements are computed before rounding. Box geometry and labels use the
existing overlay model; the viewer draws them. No windowing code enters processing.
Practice 20 remains the plain outline view; Practice 21 enables measurements using
`ContoursStage(true)`. Both read the cleaned binary mask, not the outline rendering.

## Try it

Select `contours-shapes-sample.bmp`. Compare **Practice 18 - Connected components**
on the left with **Practice 21 - Contour measurements** on the right, final stages.
Practice 20 versus Practice 21 is also useful for seeing the extra boxes and labels.

The rectangle spans 86 x 141 foreground pixels. Its pixel count is 12126, but its
contour connects the extreme pixel centers and encloses 85 x 140 = 11900 square
pixels. Its perimeter is 2 x (85 + 140) = 450 pixels. Its boundingRect still reports
width 86 and height 141 because it encloses the integer pixel coordinates.

A simpler example is a filled 10x10 square: component area 100 pixels, contour area
81 square pixels, contour perimeter 36 pixels. These are different measurements,
not a bug or a change in the object.

The ring is especially instructive: this stage uses RETR_EXTERNAL, so its enclosed
area includes the hole, and its perimeter measures only the outer boundary.
Compare with the component area, which counts only foreground pixels in the ring.
We will retrieve hole contours and their hierarchy next.

The triangle and circle are rasterized shapes; measurements follow the actual
cleaned pixel boundary rather than an ideal mathematical shape. Morphology can
change tips and corners. Values are image-space quantities, not square meters or
meters; physical measurements require calibration and scene geometry.

A single-point contour has zero geometric area and perimeter but a 1x1 bounding box.
Very thin contours may also have zero area. No area filtering is applied here.

Implementation: `src/opencv/contours_stage.cpp`.
Pipeline ID: `practice-21-contour-measurements`.
Reference: [OpenCV contour features](https://docs.opencv.org/4.13.0/dd/d49/tutorial_py_contour_features.html).
