# Practice 10 - Adaptive threshold

Original Lesson 2, section 16. Fixed thresholding uses one cutoff everywhere.
Adaptive thresholding calculates a local cutoff for each pixel. Gaussian mode
uses a weighted neighborhood average minus C. This helps under uneven illumination,
but still depends on local contrast and can respond to unwanted texture or noise.

Pipeline: Source -> Grayscale -> Gaussian blur -> Adaptive threshold.
The fixed-threshold pipeline uses the same preprocessing for a fair comparison.
The preceding blur smooths the image; the Gaussian weighting inside adaptive
thresholding calculates local cutoffs. They serve different purposes.

In `src/opencv/adaptive_threshold_stage.cpp`:
- `block_size = 11`: use an 11x11 neighborhood (odd, greater than one).
- `offset = 2.0`: subtract 2 from the local average.
- `THRESH_BINARY`: pixels above their local cutoff become white, others black.

With positive C, a uniform patch becomes white: its intensity exceeds its own
average minus C. Dark strokes become black. This is not an edge detector or an
automatic object recognizer. Thick uniform object interiors can also become white.

## Observe

Run `python scripts/create_adaptive_sample.py` to reproduce the local
`data/adaptive-lighting-sample.bmp`. It contains repeated dark outlines and strokes
against a background that becomes brighter from left to right.

Refresh the source list and select this sample. In Compare mode:
- Left: **Practice 06 - Binary threshold**, final stage.
- Right: **Practice 10 - Adaptive threshold**, final stage.

The fixed cutoff loses detail in both dark and bright areas; the local method
should preserve the strokes across more of the image. Inspect Source first to see
the illumination gradient. The generated image is controlled test data, not a photograph.

First experiment: increase offset from 2 to 8. This lowers the local cutoff, so
more pixels become white and faint dark strokes may disappear. Restore 2 afterward.
Later compare neighborhood sizes 11 and 31 while holding C fixed.

[OpenCV explanation](https://docs.opencv.org/4.13.0/d7/d4d/tutorial_py_thresholding.html).
CLI ID: `practice-10-adaptive-threshold`.
