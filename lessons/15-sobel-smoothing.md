# Practice 15 - Sobel's perpendicular smoothing

For an X derivative, Sobel combines a horizontal difference `[-1, 0, 1]`
with vertical weights `[1, 2, 1]`. This gives its familiar 3x3 kernel:

```text
-1  0  +1
-2  0  +2
-1  0  +1
```

If the horizontal differences in three neighboring rows are a, b, c, the raw
Sobel response is `a + 2*b + c`. Averaging neighboring rows reduces variation
that changes quickly vertically, while preserving a consistent vertical edge.
For Y derivatives the roles swap: difference vertically, smoothing horizontally.

## A fair comparison

Both pipelines use grayscale followed by an X derivative, with no preceding
Gaussian blur. This isolates the smoothing inside Sobel itself.

- `practice-15-x-difference`: `Sobel(..., 1, 0, 1, 1.0)`, a 3x1 horizontal kernel.
- `practice-15-sobel-smoothing`: `Sobel(..., 1, 0, 3, 0.25)`, a 3x3 kernel scaled by 1/4.

The second response is `(a + 2*b + c) / 4`. Both return the same response on an
image repeated vertically, so brightness differences are not caused by kernel gain.
For three identical rows `[0.3, 0.4, 0.5]`, both responses are 0.2. This remains a
two-pixel difference; use an additional factor of 1/2 to estimate slope per pixel.
Our actual input uses byte intensities 0..255, not normalized 0..1 values.

The display takes the absolute derivative, rounds to bytes and replicates into BGR.
Signs are lost for display only; smoothing happens on signed values first.

## Try it

Run `python scripts/create_sobel_smoothing_sample.py` if the sample is missing.
Refresh sources and select `sobel-smoothing-sample.bmp`. Enable Compare mode and
select **Practice 15 - X without smoothing** and **Practice 15 - X with smoothing**,
both at their final stages.

The upper half has a clean vertical edge. Its response should match on both sides.
The lower half adds an artificial texture alternating sign on every row. Its
extra vertical stripes disappear under the weighted average, while the real edge
remains. Near the horizontal halfway boundary the averaging mixes both regions.

This deliberately chosen texture cancels exactly: `(1 - 2 + 1) / 4 = 0`.
Ordinary noise is generally reduced, not completely removed. Fine real detail
can also be weakened. Try the real video next to see a less idealized comparison.

Implementation: `src/opencv/sobel_smoothing_stage.cpp`.
Next: Canny non-maximum suppression and hysteresis.
