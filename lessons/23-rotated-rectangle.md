# Practice 23 - Rotated bounding rectangle

An axis-aligned box stays parallel to the image axes. A minimum-area rotated box
can turn to enclose contour points more tightly. This helps estimate a tilted
object's extent and orientation, without assuming its contour is a perfect rectangle.

`Source -> HSV mask -> Opening -> Closing -> Rotated rectangle`

```cpp
const cv::Rect aligned = cv::boundingRect(contour);
const cv::RotatedRect rotated = cv::minAreaRect(contour);
cv::Point2f corners[4];
rotated.points(corners);
```

`boundingRect` returns integer x, y, width and height. `minAreaRect` returns float
center, width, height and angle. `points` calculates the four float corners. We draw
four connecting lines, including corner 3 back to corner 0, rounding only for display.
The axis-aligned rectangle uses the existing viewer overlay data; the rotated lines
are rendered into an owned debug image, with no window dependency in the stage.

## Try it

Generate `data/rotated-rectangles-sample.bmp` with `python scripts/create_rotated_sample.py`
if missing. Refresh sources, select it, and choose **Practice 23 - Rotated rectangle**.

- Dim gray: the cleaned foreground mask.
- Green box: axis-aligned boundingRect, drawn by the viewer.
- Cyan box: minAreaRect, drawn around the contour.
- Labels: rotated width x height in pixels, followed by OpenCV's returned angle in degrees.

Compare Practice 21 with Practice 23 at their final stages. The tilted elongated
rectangle makes the extra empty space in the axis-aligned box particularly obvious.
The horizontal example's boxes nearly coincide. Integer pixel bounds include the
last pixel, while float rotated extents span contour point centers, so a one-pixel
size difference for an axis-aligned shape is expected.

Do not interpret the raw angle as a unique object heading. Equivalent rectangles
can swap width and height with a 90-degree angle change, and a rectangle has no
intrinsic front/back direction. Near-square shapes can have ambiguous orientation.
Use returned corners for drawing rather than assuming an angle range or manually
interpreting width as the long side. Image y points downward. Rasterization and
morphology can slightly change fitted sizes/angles from the generated ideal shapes.

This still uses external contours. Holes do not affect an enclosing box. A box
center is not generally a shape's centroid; moments will address that next.
Point/line contours can produce zero-width or zero-height rotated rectangles.

Implementation: `src/opencv/rotated_rectangle_stage.cpp`.
Pipeline ID: `practice-23-rotated-rectangle`.
Next: moments and centroid, then circularity, then polygon approximation.
Reference: [OpenCV contour features](https://docs.opencv.org/4.13.0/dd/d49/tutorial_py_contour_features.html).
