# Practice 19 - Component area filtering

Connected components lets us make decisions about whole regions. This example
retains a component only when `stats.at<int>(label, cv::CC_STAT_AREA) >= 100`.
Area is foreground pixel count, not bounding-box area and not physical size.

`Source -> HSV mask -> Opening -> Closing -> Components: area >= 100`

The existing component stage now accepts a positive minimum area. Its default is 1,
so Practice 18 still displays every foreground component. Practice 19 passes 100.
Both pipelines analyze the same cleaned mask; neither processes the other's colors.

The stage paints rejected labels black and omits their boxes. Accepted regions keep
their exact pixel shapes, areas, original label numbers and display colors. Label
numbers may have gaps after filtering. They are still not persistent tracking IDs.
The colored output is a visualization, not a binary mask for later morphology.

## Try it

Use `hsv-cleanup-sample.bmp` and enable Compare mode:

- Left: **Practice 18 - Connected components**, final stage.
- Right: **Practice 19 - Component area filter**, final stage.

The 3416-pixel rectangle remains unchanged. The 25-pixel patch and its box disappear.
Exactly 100 pixels is accepted. To experiment, change the constructor argument in
the Practice 19 factory in `apps/common/catalogs.cpp`, rebuild, and restart the viewer:

- 25: both regions survive.
- 26 or 100: only the large rectangle survives.
- 3417: neither survives.

Opening uses local kernel geometry and can remove thin structures or change contours.
Area filtering makes one keep/reject decision for the entire connected region,
regardless of whether it is compact or long and thin. Neither knows object identity.
A real distant object may be small in pixels; resolution and distance affect which
threshold is appropriate. A small unwanted region attached to a large one is part
of that component and is not removed separately.

Implementation: `src/opencv/connected_components_stage.cpp`.
Pipeline ID: `practice-19-component-area`. Next: contours and boundary geometry.
