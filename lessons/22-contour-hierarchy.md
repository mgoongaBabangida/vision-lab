# Practice 22 - Hole contours and hierarchy

RETR_EXTERNAL omitted holes and objects nested inside them. RETR_TREE retrieves
all boundaries and describes their full nesting relationships:

```cpp
std::vector<std::vector<cv::Point>> contours;
std::vector<cv::Vec4i> hierarchy;
cv::findContours(mask, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
```

For contour i, hierarchy[i] contains four contour indices:

| Position | Meaning |
| --- | --- |
| 0 | Next sibling with the same parent |
| 1 | Previous sibling with the same parent |
| 2 | First child; follow sibling links to find other children |
| 3 | Parent |

An absent relationship is -1. These are vector indices starting at 0, not component
labels (where 0 meant background). Indices are assigned independently each frame.

Depth is the number of parent links before reaching -1. Depth 0 is an outer boundary,
depth 1 a hole, depth 2 a foreground island inside the hole, depth 3 a hole inside
that island, and so on. Even depths enclose foreground regions; odd depths enclose
background holes. Containment does not mean the foreground pixels touch.

## Try it

Generate `data/contour-hierarchy-sample.bmp` with `python scripts/create_hierarchy_sample.py`
if missing, refresh sources and select it. Compare **Practice 20 - External contours**
on the left and **Practice 22 - Contour hierarchy** on the right, final stages.

The left shows only two outer outlines. The right shows five boundaries:

```text
outer ring boundary (depth 0, yellow)
  hole (depth 1, cyan)
    island boundary (depth 2, magenta)
      hole (depth 3, cyan)
separate disk boundary (depth 0, yellow)
```

Green rectangles are viewer overlays. Their labels give contour index, role,
depth d and parent index. Follow parent numbers to reconstruct the tree. The actual
curves use the colors above. Do not assume the leftmost object receives index 0.
The source and Closing snapshots show which regions are filled foreground.

The same sample has three foreground connected components but five contours:
holes add boundaries without adding foreground objects. Try Practice 18 to compare.

This makes hole-aware geometric measurements possible: for a complete nested tree,
combine enclosed areas with alternating signs by depth. Simply summing all positive
contour areas double-counts holes. Such geometric measurements still differ from
foreground pixel counts; this exercise displays relationships without reporting
a new combined area. RETR_LIST retrieves boundaries without parent-child links,
while RETR_CCOMP flattens them into two levels; RETR_TREE preserves every level.

The processing stage uses the cleaned binary mask, not earlier outline/color output.
Existing Practice 20 and 21 continue using RETR_EXTERNAL.

Implementation: `src/opencv/contour_hierarchy_stage.cpp`.
Pipeline ID: `practice-22-contour-hierarchy`.
Next: approximating contours into polygons with approxPolyDP.
Reference: [OpenCV contour hierarchy](https://docs.opencv.org/4.13.0/d9/d8b/tutorial_py_contours_hierarchy.html).
