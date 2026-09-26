# C++ code conventions

These conventions apply to source files, headers, tests, and C++ documentation examples.
Vendored sources under `external/` retain their original formatting.

- Use **Allman braces**: scope-opening and closing braces go on their own lines.
  This includes namespaces, classes, structs, enums, functions, lambdas, loops,
  conditionals, switch statements, and try/catch blocks. Start `else`, `catch`, and
  the trailing `while` of a do/while loop on a new line too.
- Use **four spaces** for indentation, without tabs.
- Limit lines to **140 characters**, including indentation. Keep function signatures,
  calls, and expressions together when they fit. Do not wrap at a smaller arbitrary
  width. Required brace lines and separate statements still stay separate.
- Initializer-list braces are not scope braces: `FrameResult result{frame, {}, {}};`
  can remain on one line when it fits.
- Use **`auto` only in templates/generic code or when the type is obvious from the
  initializer on that same line**. Explicit casts, typed construction, and
  `std::make_unique<Type>()` qualify. A lambda literal also makes the callable
  initialization explicit. Otherwise spell out the type, including range-for
  variables and return values from ordinary function calls.

```cpp
namespace visionlab
{

struct Settings
{
    int frame_count = 300;
};

void inspect(FrameSource& source, Pipeline& pipeline)
{
    std::optional<Frame> frame = source.next();
    if (frame)
    {
        const FrameResult result = pipeline.process(std::move(*frame));
        for (const StageTiming& timing : result.timings)
        {
            print_timing(timing);
        }
    }
}

} // namespace visionlab
```

Examples of the `auto` rule:

```cpp
auto source = std::make_unique<visionlab::SyntheticSource>(300); // Type visible here.
const auto width = static_cast<std::size_t>(image.width());     // Type visible here.
visionlab::FrameResult result = pipeline.process(std::move(frame)); // Use the explicit return type.
```

## Formatter and editor support

The root `.clang-format` is the formatting authority. The configuration is verified
with clang-format 19.1.5. Prefer the same major version across machines for consistent
results. clang-format controls layout; the context-sensitive `auto` rule remains a
code-review rule and is also recorded in `AGENTS.md` for coding assistants.

Visual Studio can use the repository's `.clang-format` file. VS Code is configured
to use the C/C++ extension's clang-format support and format C++ files on save, with
a ruler at column 140. `.editorconfig` records the indentation and line limit too.

With clang-format 19 on your PATH, format or check all tracked C++ files:

PowerShell:

```powershell
$cppFiles = git ls-files '*.cpp' '*.hpp'
clang-format -i --style=file $cppFiles
clang-format --dry-run --Werror --style=file $cppFiles
```

WSL/Linux:

```bash
git ls-files -z '*.cpp' '*.hpp' | xargs -0 clang-format-19 -i --style=file
git ls-files -z '*.cpp' '*.hpp' | xargs -0 clang-format-19 --dry-run --Werror --style=file
```

See the [clang-format style reference](https://clang.llvm.org/docs/ClangFormatStyleOptions.html)
for the meaning of the formatter settings. Avoid manual alignment or line breaks
that the repository formatter immediately changes.
