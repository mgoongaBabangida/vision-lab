# Practice 25 - First complete classical detector

Target: solid orange, roughly circular regions. This combines our color, morphology,
contour, hierarchy, area, circularity and moment lessons into a detector.

`Source -> HSV mask -> Opening -> Closing -> Candidates -> Detections`

The first stage saves an owned source image and reuses HsvColorStage. The final two
views restore that source, so decisions are overlaid on the actual scene. Candidates
are analyzed from the cleaned mask, not from rendered debug images. Each factory
owns independent per-frame working data; no decoder buffers or old frames are retained.

## Detection rules

- HSV: existing H=5..25, S=100..255, V=80..255 bounds, inclusive (8-bit OpenCV HSV).
- Cleanup: opening followed by closing, each with the existing 3x3 square kernel.
- Extract contours with RETR_CCOMP. Each foreground region is a top-level candidate;
  its holes are children. Foreground islands inside holes are separate candidates.
- Reject degenerate contours with no area centroid or perimeter.
- Require contour area >=200 square pixels.
- Reject candidates with a remaining child hole. Small holes removed by closing
  no longer count; this rule describes the cleaned mask.
- Require circularity `4*pi*area/(perimeter*perimeter) >= 0.82`.

Checks use unrounded values. The candidate labels show A rounded to an integer and
C to two decimals, plus PASS or the first failed rule. Circularity is a shape measure,
not a probability or model confidence. An ideal circle scores 1 and a square pi/4;
pixel contours generally give a circle less than 1. Area is geometric contour area,
not component pixel count. For rejected rings the shown A/C describe the outer
contour; the separate hole check prevents accepting them on silhouette alone.

The final output contains accepted bounding boxes in `FrameResult::boxes` and white
centroid crosses computed using m10/m00 and m01/m00. It runs without snapshots or a
viewer. Candidate measurements live in private adapter state; this first version
uses the existing box output rather than introducing a general detection schema.

## Try it

Generate the source with `python scripts/create_detector_sample.py` if missing.
Refresh sources and select `classical-detector-sample.bmp`, then
**Practice 25 - Classical orange detector**.

| Source object | Expected decision |
| --- | --- |
| Large orange disk, top left | PASS |
| Orange square, top middle | not round |
| Elongated orange ellipse, top right | not round |
| Tiny orange disk, bottom left | too small |
| Orange ring, bottom middle | has hole |
| Green disk, bottom right | Rejected by HSV; never becomes a candidate |

In Compare mode select Practice 25 in both panes, then choose Candidates on the
left and Detections on the right. Rejected objects remain visible in the restored
source; their boxes disappear. Step back to HSV and cleanup to diagnose missing or
merged candidates. On video, the same rules run independently for every frame.

Tune `minimum_area` and `minimum_circularity` in `src/opencv/classical_detector.cpp`.
HSV thresholds live in the existing HSV stage, so editing them affects its other
pipelines too. Try C=0.75 to admit the square, or raise minimum_area to reject the disk.

This is an end-to-end detector for a constrained appearance, not semantic recognition
or tracking. Lighting, blur, perspective, occlusion, image resolution and touching
objects can change results. Test real footage and tune from its candidate view;
the supplied mixed-shape fixture validates rules, not real-world accuracy.

Pipeline ID: `practice-25-classical-detector`.
Implementation: `src/opencv/classical_detector.cpp`.
Reference: [OpenCV shape operations](https://docs.opencv.org/4.13.0/d3/dc0/group__imgproc__shape.html).
Next: experiment with detector failures and thresholds, then continue Lesson 3.
