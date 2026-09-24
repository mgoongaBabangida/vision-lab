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

## C++ code conventions

- Follow `.clang-format` and `docs/code-style.md` for all C++ code and documentation examples.
- Put scope-opening braces on a new line (Allman style), including classes, structs, namespaces,
  functions, lambdas, loops, conditionals, switch statements, and try/catch blocks.
- Keep closing braces on their own lines; `else`, `catch`, and the `while` in a do/while loop start new lines.
- Use four spaces, not tabs, and limit C++ lines to 140 characters, including indentation.
- Keep signatures, calls, and expressions on one line when they fit; avoid unnecessary wrapping.
  Scope braces still require their own lines. Initializer-list braces can stay inline.
- Use `auto` only in template/generic code or when the type is obvious from the initializer on that same line
  (for example, an explicit cast, typed construction, or `std::make_unique<Type>()`).
- Otherwise write the explicit type, including range-for variables and function-call results.
  A descriptive variable or function name alone does not make its type obvious.
- Review `auto` usage manually; clang-format handles layout, not this semantic rule.
