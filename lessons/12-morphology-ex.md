# Practice 12 - morphologyEx

Original Lesson 2, sections 22-23. This is an API exercise for operations we
already understand. Opening removes small white features; closing fills small
black gaps. OpenCV can perform either as one call:

```cpp
cv::morphologyEx(mask, result, cv::MORPH_OPEN, kernel);
cv::morphologyEx(mask, result, cv::MORPH_CLOSE, kernel);
```

These are alternatives, not two calls that always belong together.
Our reusable MorphologyExStage takes an Opening or Closing enum and chooses
the matching OpenCV operation internally. Its public header has no OpenCV types.

## Compare

Use the ball clip or Synthetic pattern. Enable Compare mode and select:

| Left | Right |
| --- | --- |
| Practice 08 - Opening | Practice 12 - Opening (morphologyEx) |
| Practice 08 - Closing | Practice 12 - Closing (morphologyEx) |

Compare one row at a time, with both panes on their final stage.
They should match: same input, 3x3 square kernel, one iteration, default anchor/borders.
Practice 08 exposes both intermediate operations; Practice 12 exposes the combined
result as one stage. The final masks match even though there are six versus five views.

If you previously changed the erosion/dilation kernels, restore both to 3x3 first.
The equivalence regression check deliberately expects matching settings.

Single-call operations keep production pipelines concise, while separate stages
remain useful for learning and inspecting intermediate results.
Next: change structuring-element geometry (rectangle, ellipse, cross, and elongated).
