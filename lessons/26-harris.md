# Practice 26 - Harris corners

`Source -> Grayscale -> Harris response -> Selected corners`

This pipeline uses the grayscale scene directly, with no HSV mask or morphology.
A local patch at a corner changes strongly when shifted in any direction. At an
edge, shifting along the edge changes little; flat patches change little everywhere.

`cornerHarris(grayFloat, response, 3, 3, 0.04)` computes a float response per pixel:
`R = det(M) - k * trace(M)^2`. M accumulates local products of Sobel X/Y derivatives.
The first 3 is the block size for accumulating products, the second 3 the Sobel
aperture, and k=0.04 weights the trace penalty. Input floats are scaled to 0..1.
There is no extra Gaussian stage in this example.

The heatmap uses red for positive R, blue for negative R, black for zero. Brightness
is sqrt(abs(R)/max(abs(R))) per frame for visibility. It is a signed visualization,
not raw response units and not directly comparable in brightness across frames.

Corner selection uses the original float response:

1. If no positive response exists, select nothing.
2. Require R > 0.01 times the strongest positive response in the frame.
3. Keep 3x3 local maxima (dilation supplies the neighborhood maximum).
4. Sort by strength, breaking ties by row then column.
5. Greedily keep points at least 8 pixels apart, up to 200 points.

A three-pixel border is excluded. Distance suppression resolves equal-score plateaus
as well as nearby maxima. Changing image contrast/content can change the relative
threshold; low-contrast noise may still be selected. This is not a confidence score.

## Try it

Generate `data/harris-corners-sample.bmp` with `python scripts/create_harris_sample.py`
if missing. Refresh sources and select **Practice 26 - Harris corners**.
Use the same pipeline in both comparison panes: response on the left, selected
corners on the right. Green 6x6 boxes mark selected pixel positions over the source.
They are point markers, not object bounding boxes; each center is the selected point.

Look for four rectangle corners, triangle tips, and checkerboard intersections.
Straight edges mostly have negative responses. The circle is rasterized and can
produce positive responses from local pixel stair steps; Harris does not know that
the intended shape is a mathematical circle. The checkerboard can dominate the
relative threshold. Try a real video next to inspect texture and noise responses.

The stage saves its own source and raw response, independently of snapshots. Final
pixels restore the original image, and corner markers are existing overlay data.
Factories have separate state and overwrite it each frame. It also runs headless.
Points have pixel precision only; no descriptors, matching, tracking or subpixel
refinement are implemented here.

Implementation: `src/opencv/harris_pipeline.cpp`. Tune quality_fraction,
minimum_distance and maximum_corners there. Pipeline ID: `practice-26-harris`.
Next: Shi-Tomasi corner selection and comparing it with Harris.
Reference: [OpenCV Harris tutorial](https://docs.opencv.org/4.13.0/dc/d0d/tutorial_py_features_harris.html).
