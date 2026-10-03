# Practice 06 - Binary threshold

Implemented by the assistant for this exercise at the learner's request.

Pipeline: Source -> Grayscale -> Gaussian blur -> Binary threshold.
Select **Practice 06 - Binary threshold** in the viewer.

In `src/opencv/binary_threshold_stage.cpp`, the image is viewed as BGR8,
converted to one gray channel, thresholded, then converted back into the
original BGR8 storage for display. Frame metadata and dimensions stay unchanged.

The key call is `cv::threshold(gray, mask, threshold, 255.0, cv::THRESH_BINARY)`.
The cutoff is the local constant `threshold = 127.0`:

- Gray values greater than 127 become 255 (white).
- Values equal to or below 127 become 0 (black).

This selects bright regions, not object identities or boundaries. On the sphere,
expect bright reflections and possibly bright background regions to survive.
Compare Gaussian blur and Binary threshold on a paused frame of the ball clip.

Next experiment: change only the cutoff to 80, then 180. Keep Gaussian settings
fixed. Lower cutoffs select more pixels; higher cutoffs select fewer. Restore 127
after comparing. The boundary-value regression check expects this baseline.

CLI ID: `practice-06-binary-threshold`.
