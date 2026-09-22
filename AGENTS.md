# Vision Lab contributor guidance

- Keep this a small teaching project; add one concept at a time.
- `visionlab_core` must remain independent of SDL, OpenGL, OpenCV, and windowing.
- Put OpenCV implementation in its optional adapter target; UI belongs in `apps/viewer`.
- Preserve a working headless build with both optional features disabled.
- Frame/debug-image ownership must be explicit; never retain borrowed decoder buffers.
- Use target-scoped CMake settings and presets; do not commit absolute machine paths.
- Treat the sibling engine and shared third_party directory as read-only dependencies.
- Keep datasets, model weights, generated results, and build outputs out of Git.
- Run appropriate CTest checks for changed behavior and document platform limitations.
