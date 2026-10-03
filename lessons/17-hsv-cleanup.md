# Practice 17 - HSV mask cleanup

This combines the operations from Lesson 2 into one segmentation pipeline:

`Source -> HSV orange mask -> Opening -> Closing`

The HSV stage selects colors (H=5..25, S=100..255, V=80..255 in OpenCV byte HSV).
Its output is a binary mask, with selected pixels white and everything else black.
Opening erodes then dilates, removing foreground structures too small for its kernel.
Closing dilates then erodes, filling small background holes and bridging narrow gaps.
Both reuse the existing 3x3 square kernel with one iteration.

No grayscale conversion precedes HSV: color is the information used for selection.
Morphology operates on the mask, not the original color picture. The three displayed
BGR mask channels are identical, so channel-wise morphology preserves a binary mask.

## Try it

Generate the source with `python scripts/create_hsv_cleanup_sample.py` if missing.
Refresh the viewer's sources and select `hsv-cleanup-sample.bmp`, then choose
**Practice 17 - HSV mask cleanup**.

1. Source: an orange rectangle has a 2-pixel gap and a 2x2 hole. There are tiny
   orange specks, a separate 5x5 orange patch, and a blue distractor.
2. HSV orange mask: blue is rejected, but all orange specks are selected.
3. Opening: isolated 1x1 and 2x2 spots disappear. The larger orange patch remains.
4. Closing: the small hole fills and the narrow split in the rectangle joins.

For a side-by-side comparison, use Practice 09 - HSV color selection on the left
and Practice 17 - HSV mask cleanup on the right, both at their final stage.
Alternatively select Practice 17 on both sides and compare Opening with Closing.

Morphology uses geometry, not object identity. The 5x5 orange patch survives because
it can contain the kernel. Real small objects can disappear, nearby objects can merge,
and larger defects can remain. Order matters: opening first removes tiny spots before
closing could connect them to the object. It is not a universal best order.

This output is a cleaned segmentation mask, not yet a list of detected objects.
Lesson 3 will introduce connected components and contours to obtain individual
regions, areas, centroids and bounding boxes. Color selection is useful when appearance
is constrained; it does not provide the semantic recognition of a learned detector.

This completes our planned Lesson 2 practical review. Next: connected components.
Pipeline ID: `practice-17-hsv-cleanup`. Composition: `apps/common/catalogs.cpp`.
