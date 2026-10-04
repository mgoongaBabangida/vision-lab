# Practice 28 - First nearest-neighbor tracker

Detection asks: where are objects in this frame? Tracking asks: which detection is
probably the same object that appeared in the previous frame?

This pipeline runs the existing solid-orange round-object detector, then adds a
small stateful stage. It uses bounding-box centers (not contour centroids).

1. Store each previous detection's center and ID.
2. Compute Euclidean distances from previous centers to current centers.
3. Discard pairs farther than 50 pixels. Sort remaining pairs by distance.
4. Greedily accept shortest pairs, using each old track and new detection only once.
5. Give unmatched detections new IDs. Retire unmatched old tracks immediately.
6. Store the current centers and IDs for the next frame.

The tracker changes box labels to `ID n (new)` or `ID n (matched)`. It leaves image
pixels and box geometry unchanged. `NearestNeighborStage` uses no OpenCV or UI code;
its deliberately simple input contract is that all incoming boxes are detections.
A future detection model can replace that teaching contract with typed detection and
track records without changing the association idea.

## Experiment

Run `python scripts/create_tracker_sample.py` once (standard library only). It creates
`data/tracker-sample.avi`, about 40 MiB, and refuses to overwrite an existing file.
The sample is generated locally; it is not real footage. Three orange circles move
on separate rows. The bottom circle disappears at frame 70 and returns at frame 100.

Refresh viewer sources, select `tracker-sample.avi`, choose **Practice 28 -
Nearest-neighbor tracker**, and select the final **Nearest-neighbor IDs** stage.
Press Play, or advance one frame at a time. Each circle should keep its ID while
visible; the returning circle receives a new ID. For comparison, use Practice 25
on the other side to see the same frame with detections but no IDs.

Previous/Next stage only inspects snapshots. Previous/Next frame within viewer
history reuses cached results. Restart creates a fresh pipeline and restarts IDs.
Changing the source or selecting the tracker mid-video also starts fresh IDs.
The stage additionally resets if a direct caller changes image dimensions or passes
a nonconsecutive frame index. IDs are local to one uninterrupted pipeline sequence.

## Limits to observe

This is a proto-tracker: no motion prediction, appearance descriptor, lost-track
buffer, or re-identification. A single missed detection loses the ID. Fast motion
outside the gate creates a new ID. Nearby objects or crossings can swap IDs, and
merged detections confuse the association. Shortest-first greedy assignment is not
globally optimal. Camera motion also moves the measured centers; these are image
positions, not physical world motion. The 50-pixel gate assumes modest motion between
consecutive processed frames and depends on resolution and frame rate.

Try decreasing the gate passed to `NearestNeighborStage` in `apps/common/catalogs.cpp`
and rebuilding. At what threshold do normal movements start receiving new IDs?
