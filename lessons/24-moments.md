# Practice 24 - Moments and centroid

Moments summarize how a region's area is distributed across image coordinates.
The centroid is the balance point of a uniformly filled region. It gives a useful
position derived from the whole shape, rather than just its extreme coordinates.

`Source -> HSV mask -> Opening -> Closing -> Moments and centroid`

```cpp
const cv::Moments moments = cv::moments(contour);
const double cx = moments.m10 / moments.m00;
const double cy = moments.m01 / moments.m00;
```

For contour moments, m00 is geometric enclosed area; m10 and m01 are integrals of
x and y over that area. Dividing by area gives mean x and y. These are not the
arithmetic means of the stored contour vertices: vertex spacing need not be uniform.
The implementation checks m00 before division. A point or line contour has no area
centroid; it receives an 'undefined' label and no centroid cross.

OpenCV can also compute moments of an image. For `moments(mask, true)`, each nonzero
pixel has unit weight, so m00 is foreground pixel count and the centroid averages
their coordinates. Here we deliberately use contour moments, continuing Practice 21.
Image moments without the binary flag weight pixels by their intensity instead.

## Try it

Generate `data/moments-centroid-sample.bmp` with `python scripts/create_moments_sample.py`
if missing. Refresh sources and choose **Practice 24 - Moments and centroid**.

- Yellow cross: contour area centroid, C=(x,y).
- Cyan circle: center of the axis-aligned bounding extent.
- Green box: the existing bounding-box overlay.
- Gray fill: the cleaned mask. m00 in the label is contour area in square pixels.

The rectangle's markers coincide. The triangle's centroid moves toward its broad
corner, away from the bounding-box center. The L shape shows that a centroid can
lie outside the foreground: its balance point need not be a filled pixel.

Compare Practice 23 and Practice 24 on this same source to distinguish enclosing
rectangle geometry from area distribution. Shapes are measured after morphology,
so a clipped tip can slightly shift the triangle's measured centroid.

The cyan marker uses `x + (width - 1)/2`, and similarly for y: the midpoint of the
extreme pixel-center coordinates. The viewer's pixel-inclusive box edges can differ
by half a pixel from this geometric midpoint. All coordinates remain floating point
until marker drawing, where they are rounded. Label values use one decimal place.

This uses RETR_EXTERNAL: moments describe the area enclosed by each outer boundary,
including holes. For an asymmetric hole, the actual foreground centroid can differ.
Hole-aware moments require combining parent/child contributions, or measuring the
relevant binary region. No tracking or physical-world calibration is implied.

Implementation: `src/opencv/moments_stage.cpp`.
Pipeline ID: `practice-24-moments`.
Next: circularity, then polygon approximation.
Reference: [OpenCV contour features](https://docs.opencv.org/4.13.0/dd/d49/tutorial_py_contour_features.html).
