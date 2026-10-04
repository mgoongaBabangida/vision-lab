# Local input data

Kernel geometry sample: run `python scripts/create_kernel_sample.py` to create
`data/kernel-shapes-sample.bmp`, then Refresh. This original 160x100 binary chart
shows single pixels and broken horizontal/vertical lines for Practice 13.

Otsu sample: run `python scripts/create_otsu_sample.py` to create
`data/otsu-dim-sample.bmp`, then Refresh. This original generated chart has gray
shapes on a dark background; both intensity groups fall below the fixed cutoff of 127.

Adaptive threshold sample: run `python scripts/create_adaptive_sample.py` to create
`data/adaptive-lighting-sample.bmp`, then Refresh. This original generated chart
contains dark strokes across an illumination gradient for fixed/adaptive comparison.

HSV lesson sample: run `python scripts/create_hsv_sample.py` from the repository
root to create `data/hsv-color-sample.bmp`, then click Refresh in the viewer.
The generated six-circle chart is original project sample data; no external media is used.

Place local images and videos here. Everything except this README is ignored by Git.
The viewer lists eligible files alongside Synthetic pattern; click Refresh after adding files.

## Bouncing ball sample

- Local filename: `bouncing-ball.webm`
- Title/creator: Bouncing Ball, ScienceCOLA (2013).
- Source: https://commons.wikimedia.org/wiki/File:Bouncing_Ball.webm
- Original publication: https://www.youtube.com/watch?v=A5z0i2SdqDk
- License: CC BY 3.0, https://creativecommons.org/licenses/by/3.0/
- Downloaded unchanged from Wikimedia Commons; only the local filename was changed.
- Approximately 26 seconds, 640x480, 1.03 MiB. Starts with a black frame; press Play.
- SHA-1: `63a7d2d42be52bfc13fcf3643605b39841ae52d5`

Download from the repository root on another machine (use `curl.exe` in Windows PowerShell):

```sh
curl -fL "https://upload.wikimedia.org/wikipedia/commons/3/32/Bouncing_Ball.webm" -o data/bouncing-ball.webm
```

Select the file in the Source dropdown, choose a practice pipeline, and press Play.
The media stays local rather than increasing the repository size.

## Short motion-only cut (recommended)

`bouncing-ball-motion.avi` contains decoded frames 500-655 of the sample above:
approximately 6.2 seconds at 25 FPS, 640x480, MJPEG video without audio.
Changes: removed the introduction and re-encoded the video. Same ScienceCOLA
attribution and CC BY 3.0 license apply. The original download is kept locally
at `out/bouncing-ball-original.webm`.

To reproduce the cut with FFmpeg after downloading the original as above:

```sh
ffmpeg -i data/bouncing-ball.webm -vf "trim=start_frame=500:end_frame=656,setpts=PTS-STARTPTS" -an -c:v mjpeg -q:v 3 data/bouncing-ball-motion.avi
```

Select `bouncing-ball-motion.avi` in the viewer to skip the text introduction.


Gradient-direction sample: run `python scripts/create_gradient_sample.py` to create
`data/gradient-direction-sample.bmp` (rectangle, circle, and lower-contrast triangle).
Use Practice 14 to inspect edge direction colors; the generated BMP stays untracked.

Sobel smoothing sample: run `python scripts/create_sobel_smoothing_sample.py` to create
`data/sobel-smoothing-sample.bmp`. Practice 15 compares a clean vertical edge (top)
and the same edge with alternating-row interference (bottom). The BMP stays untracked.

Canny hysteresis sample: run `python scripts/create_canny_sample.py` to create
`data/canny-hysteresis-sample.bmp`. Practice 16 compares connected strong/weak
edges on the left with a separate weak rectangle on the right. The BMP stays untracked.

HSV cleanup sample: run `python scripts/create_hsv_cleanup_sample.py` to create
`data/hsv-cleanup-sample.bmp`. Practice 17 removes tiny orange spots and repairs
a small hole and narrow gap. A larger orange patch survives; a blue distractor is rejected.
The generated BMP stays untracked.

Contours sample: run `python scripts/create_contours_sample.py` to create
`data/contours-shapes-sample.bmp`. Practice 20 traces the outer boundaries of
an orange rectangle, ring and triangle. The generated BMP stays untracked.

Contour hierarchy sample: run `python scripts/create_hierarchy_sample.py` to create
`data/contour-hierarchy-sample.bmp`. Practice 22 shows an outer ring, its hole,
a nested island with another hole, and a separate disk. The BMP stays untracked.

Rotated rectangles sample: run `python scripts/create_rotated_sample.py` to create
`data/rotated-rectangles-sample.bmp`. Practice 23 compares enclosing rectangles
on horizontal and tilted orange shapes. The generated BMP stays untracked.

Moments sample: run `python scripts/create_moments_sample.py` to create
`data/moments-centroid-sample.bmp`. Practice 24 compares area centroids and
box centers on a rectangle, right triangle and L shape. The BMP stays untracked.

Classical detector sample: run `python scripts/create_detector_sample.py` to create
`data/classical-detector-sample.bmp`. Practice 25 detects the large orange disk
among square, ellipse, small disk, ring and wrong-color distractors. The BMP stays untracked.

Harris sample: run `python scripts/create_harris_sample.py` to create
`data/harris-corners-sample.bmp`. Practice 26 compares corner responses on a
rectangle, triangle, circle and checkerboard. The generated BMP stays untracked.

## Feature matching sample

Run `python scripts/create_feature_matching_sample.py` to create
`feature-matching-sample.bmp`, deterministic varied texture made locally with no
external assets or dependencies. The script refuses to overwrite an existing file.
Use it with Practice 27 SIFT/ORB; a textured real photograph also works.

## Proto-tracker sample

Run `python scripts/create_tracker_sample.py` to generate `tracker-sample.avi`
(180 frames, 30 FPS, about 40 MiB). Three orange circles move; the bottom circle
disappears at frame 70 and returns at frame 100. Use Practice 28 to see persistent
IDs and a new ID after the interruption. Standard library only, no external assets.
The generator refuses to overwrite an existing file.
