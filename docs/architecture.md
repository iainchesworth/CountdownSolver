# Architecture

All game logic lives in a GUI-free library, `countdown::solver`; a thin
**Qt 6 Quick (QML)** application provides the GUI, wired to the library
through a small `Solver` backend
([`src/app/solver.hpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/app/solver.hpp)).
The QML/visual design lives in
[`src/app/qml/`](https://github.com/iainchesworth/CountdownSolver/tree/develop/src/app/qml)
— see
[`src/app/DESIGN_SPEC.md`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/app/DESIGN_SPEC.md)
for the colour tokens, type scale, and layout rules the QML follows.

## Layout

```
src/lib/     countdown::solver — all game logic (no GUI, no platform code)
src/app/     Qt 6 Quick (QML) GUI: solver.* backend + qml/ design
src/app/platform/{windows,macos,linux,android,ios}/   one impl each, chosen by CMake
tests/unit/          unit tests
tests/integration/   integration tests
cmake/               shared CMake modules (strict warnings-as-errors)
deps/vcpkg/          vendored vcpkg (git submodule, pinned)
```

## Design highlights

This project deliberately uses modern **C++23** throughout:

| Feature | Where |
| --- | --- |
| `std::expected` + monadic `and_then` / no exceptions | [`error.hpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/include/countdown/error.hpp), [`numbers_game.cpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/src/numbers/numbers_game.cpp) |
| Deducing `this` fluent builders | [`numbers_game.hpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/include/countdown/numbers/numbers_game.hpp), [`letters_game.hpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/include/countdown/letters/letters_game.hpp) |
| `if consteval` compile-time vs runtime branching | [`frequencies.hpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/include/countdown/letters/frequencies.hpp) |
| `views::zip` / `stride` / `enumerate` / `transform` + `ranges::to` | [`dictionary.cpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/src/letters/dictionary.cpp), [`solution.cpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/src/numbers/solution.cpp) |
| Platform code selected by CMake, **no `#ifdef`** | [`src/app/platform/`](https://github.com/iainchesworth/CountdownSolver/tree/develop/src/app/platform) |
| Build-time git version stamp | [`GenerateVersion.cmake`](https://github.com/iainchesworth/CountdownSolver/blob/develop/cmake/GenerateVersion.cmake), [`version.hpp.in`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/include/countdown/version.hpp.in) |

Because the solver has no dependency on Qt or any GUI toolkit, it's
unit-tested entirely on its own (see [Testing & coverage](testing.md)) and
could be dropped into a different frontend without touching a line of
game logic.

## Design decisions

The choices above aren't arbitrary — here's the reasoning, for anyone
extending the code and deciding whether to follow the same pattern or
break from it.

**`std::expected` over exceptions.** Every fallible library operation
returns `Result<T>` — an alias for `std::expected<T, SolveError>`
([`error.hpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/include/countdown/error.hpp))
— rather than throwing. Call sites compose results with the C++23 monadic
operations (`and_then`, `transform`, `or_else`) instead of try/catch, and
every possible failure is visible in a function's signature. New fallible
operations should extend `SolveError` and return `Result<T>`, not throw.

**Deducing-`this` fluent builders.** `NumbersGame` and `LettersGame`
([`numbers_game.hpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/include/countdown/numbers/numbers_game.hpp),
[`letters_game.hpp`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/lib/include/countdown/letters/letters_game.hpp))
use C++23's deducing `this` so each `with_*` setter is written once as a
template and still returns the right reference category (const/non-const,
lvalue/rvalue) for whatever it was called on — no hand-duplicated
overload sets.

**Platform code selected by CMake, not `#ifdef`.**
[`src/app/CMakeLists.txt`](https://github.com/iainchesworth/CountdownSolver/blob/develop/src/app/CMakeLists.txt)
picks exactly one of `platform/{windows,macos,linux}/platform_*.cpp` at
configure time based on the target OS; each implements the same
`platform.hpp` interface. App code calls that interface and never
branches on OS at compile time — a new platform means adding one file,
not sprinkling `#ifdef`s through existing ones.

**GUI/library split.** `countdown::solver` has zero Qt or GUI dependency
by design, which is what makes it independently unit-testable (see
[Testing & coverage](testing.md)) and reusable behind a different
frontend. The Qt Quick app is deliberately kept thin — a `Solver` backend
plus QML presentation — so game logic changes never touch UI code and
vice versa.

**Dependencies: vcpkg vendored, Qt prebuilt.** vcpkg is a pinned git
submodule rather than a system install, so the C++ dependency set
(currently just Catch2) is reproducible without machine-wide setup. Qt is
installed as an official prebuilt package instead of built from source —
seen in [Getting started](getting-started.md) and
[CI & dependencies](ci.md) — because a from-source Qt build is both huge
and, on Windows, prone to exceeding `MAX_PATH`. Together these keep "clone
and build" close to a one-command experience across all three platforms.
