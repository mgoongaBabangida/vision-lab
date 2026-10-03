# Practice 20 - External contours

Connected components groups foreground pixels into regions. A contour describes a
boundary as an ordered sequence of points. This representation lets us later measure
perimeter, approximate shapes and examine how boundaries enclose one another.

`Source -> HSV mask -> Opening -> Closing -> External contours`

This pipeline reads the cleaned binary mask directly. It does not consume Practice
18's colored component image or apply Practice 19's area filter.

## The code

`cvtColor` converts our three-channel BGR mask into a one-channel byte mask.

```cpp
std::vector<std::vector<cv::Point>> contours;
cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
```

The outer vector contains contours; each inner vector contains ordered `(x, y)`
points. `RETR_EXTERNAL` returns only outermost boundaries: holes and boundaries
nested inside them are omitted. `CHAIN_APPROX_SIMPLE` stores endpoints of straight
horizontal, vertical and diagonal runs. An ideal axis-aligned rectangle needs four
points; a rasterized circle needs many. This is not a general triangle recognizer
or polygon simplification with an adjustable tolerance.

```cpp
view.setTo(cv::Scalar(0, 0, 0));
cv::drawContours(view, contours, -1, cv::Scalar(80, 230, 255), 1, cv::LINE_8);
```

Clear the display to black, then draw every contour (`-1`) in yellow BGR color,
with a one-pixel line. The renderer connects successive stored points, so a compact
point list still draws continuous outlines. No bounding boxes obscure the curves.
The output is a debug visualization; the earlier mask remains an owned snapshot.

## Try it

Run `python scripts/create_contours_sample.py` if the sample is missing.
Refresh sources and select `contours-shapes-sample.bmp`.
Compare **Practice 18 - Connected components** on the left with
**Practice 20 - External contours** on the right, both at their final stage.

The filled rectangle, ring and triangle become three outer outlines.
The ring's inner boundary is deliberately absent because of `RETR_EXTERNAL`.
Its hole remains visible in the preceding Closing stage and component view.
Opening and closing can slightly change corners before contours are extracted.

Unlike Canny, this operation traces boundaries of already-selected foreground
regions; it does not measure brightness gradients or use hysteresis thresholds.

In Visual Studio, break after `findContours` to inspect `contours.size()` and each
`contours[i]` point vector. Next: contour perimeter and geometric area, followed
by hole retrieval and hierarchy.

Implementation: `src/opencv/contours_stage.cpp`.
Pipeline ID: `practice-20-contours`.
Reference: [OpenCV contour introduction](https://docs.opencv.org/4.13.0/d4/d73/tutorial_py_contours_begin.html).
