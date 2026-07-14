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
