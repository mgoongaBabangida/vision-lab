# Practice 01 - BGR to grayscale

Completed by the learner in `src/opencv/grayscale_stage.cpp`.
The exercise instructions below remain as a reference. The implementation is now
covered by known-pixel checks for BGR interpretation, equal output channels, frame
metadata, and preservation of the source snapshot.

## The idea

Our `Image` owns tightly packed BGR8 pixels: three unsigned bytes per pixel, in
blue/green/red order. A temporary `cv::Mat` can describe that existing buffer by
supplying rows, columns, type, data pointer, and row stride. This view borrows the
pixels; it must not outlive the `Image`.

`CV_8UC3` describes 8-bit unsigned values with three channels. It does not itself
specify BGR versus RGB. Rows are height; columns are width. Stride is the number
of bytes between the starts of consecutive rows. Read OpenCV's
[Mat introduction](https://docs.opencv.org/4.13.0/d6/d6d/tutorial_mat_the_basic_image_container.html)
and the external-data constructor comments in `opencv2/core/mat.hpp`.

Grayscale combines the channels into one value, approximately:

`gray = 0.114 * B + 0.587 * G + 0.299 * R`

It is a weighted sum, not the arithmetic average. Green contributes more than red,
and red more than blue. See OpenCV's
[color conversions](https://docs.opencv.org/4.13.0/de/d25/imgproc_color_conversions.html).

## Your implementation

1. Obtain a reference to `result.frame.image`. Create a temporary `cv::Mat` view
   using its `height()`, `width()`, `data()`, `stride_bytes()`, and `CV_8UC3`.
2. Create a separate empty `cv::Mat` for grayscale. Use `cv::cvtColor` with
   `cv::COLOR_BGR2GRAY` to fill it from the BGR view.
3. Our current image/viewer contract still requires three channels. Use a second
   conversion, `cv::COLOR_GRAY2BGR`, to write the grayscale result into the original
   BGR view. Each pixel then contains three equal values. This does not recover color.

Keep the first conversion's destination separate: converting the BGR view directly
to one channel can reallocate its `cv::Mat` buffer rather than update our `Image`.
No pixel loop, new UI code, or changes to frame index/timestamp are needed.

## Run and inspect

Build Debug/x64, press F5, and select **Practice 01 - Grayscale**.
Start with **Synthetic pattern**. Use **Source** and **Next stage** to compare the
same frame before and after processing. Then try a color photograph.

Put a breakpoint in `process`. After creating the view, inspect `rows`, `cols`,
`channels()`, and `step[0]`. After conversion, grayscale should have one channel
and unchanged dimensions. The final BGR image should have equal channel values.

Check your understanding: which becomes brighter, pure blue or pure green?
For equal maximum channel values, expect blue about 29, green about 150, red about
76; white remains 255 and black 0. These distinguish a correct BGR conversion from
an accidental RGB conversion.

Next: [Practice 02 - Mean blur](02-mean-blur.md), following original Lesson 2, section 1.

CLI entry: `visionlab_cli --pipeline practice-01-grayscale --frames 1` (OpenCV build).
