# Practice 08 - Opening and closing

Both pipelines start with Source -> Grayscale -> Gaussian blur -> Binary threshold.
They reuse the existing erosion and dilation stages with the same centered 3x3
square kernel and one iteration per operation. Their order makes the difference:

| Pipeline | After threshold | Typical effect on white regions |
| --- | --- | --- |
| Practice 08 - Opening | Erosion -> Dilation | Removes small specks and thin protrusions; can break narrow connections. |
| Practice 08 - Closing | Dilation -> Erosion | Fills small holes and gaps; can connect nearby regions. |

These are not inverse operations. In opening, dilation grows the regions that
survived erosion; it cannot recover an isolated speck that vanished. Closing's
erosion trims the expanded regions but need not reopen gaps that were filled.
Neither operation guarantees preservation of the original object shape.

## Compare in the viewer

1. Select the ball clip and pause on a useful frame.
2. Enable Compare mode: Opening on the left, Closing on the right.
3. Choose Binary threshold in both panes to see their shared starting mask.
4. Step forward once in each pane to compare erosion with dilation.
5. Step forward again to inspect the complete opening and closing results.

The final stage is labeled Dilation for Opening and Erosion for Closing because
the viewer exposes the actual individual operations. Each pipeline has six views,
including Source. Keeping the operations separate makes their effects inspectable.

Look for a small isolated white reflection and a small black gap inside a white
region. Which operation removes each? Changes can be subtle with a 3x3 kernel.
If experimenting with 5x5, change both stage kernels together, then restore 3x3.
Keep threshold and Gaussian settings fixed while comparing.

Pipeline composition lives in `apps/common/catalogs.cpp`. No new pixel-processing
code is necessary. CLI IDs: `practice-08-opening` and `practice-08-closing`.
