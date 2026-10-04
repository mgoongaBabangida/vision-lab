# Practice 27 - SIFT and ORB matching

A keypoint tells us where a distinctive image feature is. Its descriptor summarizes
nearby appearance, allowing us to find a corresponding feature in another image.
SIFT uses gradient histograms and floating-point descriptors; ORB uses oriented
brightness comparisons and binary descriptors. Both handle rotation and scale changes
within practical limits. This exercise uses one frame and a transformed copy, not
tracking between video frames.

## Try it

Run `python scripts/create_feature_matching_sample.py` once, then refresh sources in
the viewer and select `feature-matching-sample.bmp`. Enable Compare mode, selecting
**Practice 27 - SIFT matching** and **Practice 27 - ORB matching**. Set both sides to
the same stage and advance together. Each pipeline shows its own original/target pair.
You can also use a textured photograph or pause a real video frame.

1. **Source:** the unmodified source frame.
2. **Image pair:** resize to at most 640 pixels on the longest side; rotate a copy
   by +25 degrees and scale it to 0.80 around the center. The target canvas keeps
   the same size, so some corners can clip; exposed borders are gray.
3. **Keypoints and descriptors:** `detectAndCompute` finds features independently
   in both images. Circles show feature size and spokes show orientation.
4. **Descriptor matches:** find the two nearest descriptors for every source feature.
   SIFT uses Euclidean (L2) distance; default ORB uses Hamming distance (differing bits).
   Keep a match only if its distance is below 0.75 times the runner-up's distance.
   This rejects ambiguous matches; it does not enforce one-to-one or mutual matches.
5. **RANSAC homography inliers:** estimate a common geometric mapping from at least
   four matches. Keep matches within the 3-pixel RANSAC reprojection threshold.
   A homography is suitable for this synthetic image transformation; real UAV scenes
   with depth and camera translation may not share one homography.

Green lines connect matched points; at most 60 best descriptor matches are drawn
for readability. Header counts include all matches. `true error` is the median
inlier correspondence error against our **known** rotation/scale, in working-image
pixels. It is an independent accuracy check, not error against the fitted homography.
Blank or very small inputs may have no features and display `no valid model`.

Compare feature coverage, retained matches and true error. More matches alone do not
mean greater accuracy. Stage timings include visualization and are not a rigorous
SIFT-versus-ORB speed benchmark. Repeated checkerboards and plain synthetic shapes can
be ambiguous; the sample uses varied texture intentionally.

## Code

`add_feature_matching` adds four stages with private shared working data. Descriptors
and original images stay separate from rendered debug images. Snapshots are optional;
the same pipeline works headless. The optional `FeatureMatchReport` exposes counts
and known-transform error for tests or other callers. Each factory creates fresh state.

Next experiment: change `rotation_degrees` or `scale_factor` in
`src/opencv/feature_matching.cpp`, rebuild, and compare which features still match.
The image header displays the current transform constants.

References: [OpenCV matching](https://docs.opencv.org/4.13.0/dc/dc3/tutorial_py_matcher.html)
and [homography](https://docs.opencv.org/4.13.0/d1/de0/tutorial_py_feature_homography.html).
