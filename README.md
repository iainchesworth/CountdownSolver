# CountdownSolver

[![CI](https://github.com/iainchesworth/CountdownSolver/actions/workflows/ci.yml/badge.svg)](https://github.com/iainchesworth/CountdownSolver/actions/workflows/ci.yml)
[![CodeQL](https://github.com/iainchesworth/CountdownSolver/actions/workflows/codeql.yml/badge.svg)](https://github.com/iainchesworth/CountdownSolver/actions/workflows/codeql.yml)
[![OSV-Scanner](https://github.com/iainchesworth/CountdownSolver/actions/workflows/osv-scanner.yml/badge.svg)](https://github.com/iainchesworth/CountdownSolver/actions/workflows/osv-scanner.yml)
[![Zizmor](https://github.com/iainchesworth/CountdownSolver/actions/workflows/zizmor.yml/badge.svg)](https://github.com/iainchesworth/CountdownSolver/actions/workflows/zizmor.yml)
[![OpenSSF Scorecard](https://api.scorecard.dev/projects/github.com/iainchesworth/CountdownSolver/badge)](https://scorecard.dev/viewer/?uri=github.com/iainchesworth/CountdownSolver)
[![Docs](https://img.shields.io/badge/docs-published-2f7d7b)](https://iainchesworth.github.io/CountdownSolver/)
[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](docs/architecture.md)
[![License: GPL v3+](https://img.shields.io/badge/license-GPL--3.0--or--later-blue)](LICENSE)

A cross-platform desktop solver for the two puzzle games from the UK TV
show **[Countdown](https://en.wikipedia.org/wiki/Countdown_(game_show))**:

- **Numbers** — given six numbers and a target (100–999), find an
  arithmetic expression (`+ - x /`, each number at most once, only
  positive whole intermediate results, exact division) that reaches or
  gets closest to the target.
- **Letters** — given nine letters, find the longest dictionary word(s)
  that can be spelled from them.

It'll also generate and solve the show's third game, the **Conundrum**
(unscramble nine letters into one word).

![Numbers page, solved](docs/assets/screenshots/numbers.png)

More screenshots and the full docs live at
**[iainchesworth.github.io/CountdownSolver](https://iainchesworth.github.io/CountdownSolver/)**.

## Under the hood

All game logic lives in a GUI-free library (`countdown::solver`); a thin
**Qt 6 Quick (QML)** application provides the GUI, wired to the library
through a small `Solver` backend (`src/app/solver.hpp`). See
[Architecture](https://iainchesworth.github.io/CountdownSolver/architecture/)
for the modern C++23 features it leans on
(`std::expected`, ranges, deducing `this`, and more) and how the source
tree is laid out.

## Quick start

```sh
git clone --recurse-submodules https://github.com/iainchesworth/CountdownSolver
cd CountdownSolver
cmake --preset windows-msvc-debug -DCMAKE_PREFIX_PATH=<qt>/6.8.3/<arch>
cmake --build --preset windows-msvc-debug
./build/windows-msvc-debug/bin/countdownsolver
```

That needs a C++23 compiler, CMake 4+, Ninja, and Qt 6.8 — see
[Getting started](https://iainchesworth.github.io/CountdownSolver/getting-started/)
for the full requirements and every platform's preset. vcpkg is vendored
as a submodule and bootstraps itself automatically, so there's nothing to
install for it.

Just want the solver library and tests, no Qt/GUI?

```sh
cmake --preset linux-gcc-debug -DCOUNTDOWN_BUILD_APP=OFF
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

## Documentation

| | |
| --- | --- |
| [How to play](https://iainchesworth.github.io/CountdownSolver/how-to-play/) | Each round explained, and what the results mean |
| [Getting started](https://iainchesworth.github.io/CountdownSolver/getting-started/) | Requirements, cloning, first build |
| [Building & packaging](https://iainchesworth.github.io/CountdownSolver/building/) | Every preset, sanitizers, `cpack` |
| [Testing & coverage](https://iainchesworth.github.io/CountdownSolver/testing/) | TDD, test layers, the 80% coverage gate |
| [Architecture](https://iainchesworth.github.io/CountdownSolver/architecture/) | C++23 highlights, source layout |
| [Contributing](https://iainchesworth.github.io/CountdownSolver/CONTRIBUTING/) | Branching model, code style, PR process |

## Contributing

Contributions are welcome — see
[CONTRIBUTING.md](https://iainchesworth.github.io/CountdownSolver/CONTRIBUTING/)
for the branching model (this project follows
[gitflow](https://nvie.com/posts/a-successful-git-branching-model/):
`develop` for day-to-day work, `main` for tagged releases only) and how
PRs get reviewed. Please also read the
[Code of Conduct](https://iainchesworth.github.io/CountdownSolver/CODE_OF_CONDUCT/).

## License

GPL-3.0-or-later. See [LICENSE](LICENSE).
