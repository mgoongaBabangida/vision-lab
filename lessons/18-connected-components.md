# Lesson 3, Practice 18 - Connected components

A binary mask says which pixels belong to the foreground. Connected components
group touching foreground pixels into separate regions and assign each a label.
We use eight-connectivity: horizontal, vertical and diagonal neighbors all connect.
Four-connectivity would exclude diagonal-only contact.

`Source -> HSV mask -> Opening -> Closing -> Connected components`

## What the OpenCV operation returns

`connectedComponentsWithStats(mask, labels, stats, centroids, 8, CV_32S)` returns:

- A count including background label 0, so foreground count is `count - 1`.
- A CV_32S label image: one integer component ID per pixel.
- A CV_32S statistics table: left, top, width, height and foreground pixel area per label.
- A CV_64F centroid table: mean x and y of a component's pixels. We will explore these later.

The input is a single-channel byte mask; zero is background, nonzero is foreground.
We skip label 0, color foreground labels for inspection, and store bounding boxes
using the project's existing overlay data. The viewer draws boxes and area labels.
All processing also works without a window. Label colors repeat after six components.

Area counts foreground pixels, not bounding-box area. A region with a hole therefore
has less area than its enclosing rectangle. A component means a connected region,
not necessarily one physical object: touching objects merge, and a broken object
may split. IDs are assigned independently each frame and are not tracking IDs.

## Try it

Select `hsv-cleanup-sample.bmp` from Practice 17 and choose
**Practice 18 - Connected components**. Select the final stage.
If missing, regenerate the image with `python scripts/create_hsv_cleanup_sample.py`.

You should see two colored components and boxes:

- The repaired rectangle: 61 x 56, area 3416 pixels.
- The surviving orange patch: 5 x 5, area 25 pixels.

Use Compare mode with Practice 17 on the left and Practice 18 on the right to
compare the clean binary mask with the labeled regions. Earlier snapshots have
no boxes; the boxes appear only after component extraction.

The little patch is counted intentionally. No area filtering is applied yet.
Next we can use the measured area to reject small regions, then explore contours.

Implementation: `src/opencv/connected_components_stage.cpp`.
Pipeline ID: `practice-18-connected-components`.
Reference: [OpenCV connected components and statistics](https://docs.opencv.org/4.13.0/d3/dc0/group__imgproc__shape.html).
