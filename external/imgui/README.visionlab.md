# Dear ImGui dependency

This is a pinned source snapshot of the engine's Dear ImGui 1.89.2 docking bundle,
including its matching SDL2 and OpenGL3 backends. The files were copied together;
this is not a claim that the engine snapshot is an unmodified upstream release.
The viewer builds these sources directly and does not require the engine checkout.

Upstream: https://github.com/ocornut/imgui
License: MIT, included in LICENSE.txt from the upstream v1.89.2 tag.
Snapshot date: 2026-09-24. Checksums are recorded in SHA256SUMS.txt.

Third-party sources retain their original formatting. Update core and backends as
one reviewed set; do not mix backend files from a newer release into this copy.

Local integration patch: the SDL backend's three engine-specific `SDL/include/`
include paths were changed to standard SDL header names supplied by `SDL2::SDL2`.
