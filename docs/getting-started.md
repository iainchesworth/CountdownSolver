# Getting started

## What you'll need

- A **C++23** compiler: GCC 14+, Clang 18+, or MSVC 19.40+ (VS 2022 17.10+ / VS 18).
- **CMake 4.0+** and **Ninja**.
- **Git** — vcpkg is vendored as a submodule at
  [`deps/vcpkg`](https://github.com/iainchesworth/CountdownSolver/tree/develop/deps/vcpkg),
  so there's nothing to install machine-wide for it. It provides
  [Catch2](https://github.com/catchorg/Catch2) for the test suite, pinned to
  the submodule commit.
- **Qt 6.8** — only needed if you want to build the GUI app (the default).
  Qt ships as an official prebuilt package rather than being built from
  source (a from-source Qt build is huge, and on Windows its build tree
  overruns `MAX_PATH`). Install it with
  [`ci/install-qt.ps1`](https://github.com/iainchesworth/CountdownSolver/blob/develop/ci/install-qt.ps1)
  (Windows) or
  [`ci/install-qt.sh`](https://github.com/iainchesworth/CountdownSolver/blob/develop/ci/install-qt.sh)
  (Linux/macOS) — both use [aqtinstall](https://github.com/miurahr/aqtinstall).
  See [CI & dependencies](ci.md) for details.
- **Mobile builds only** (Android/iOS, tablet + landscape-only — see
  [Building & packaging](building.md)): an Android SDK + a pinned NDK
  version plus a Qt-for-Android kit, or Xcode + a Qt-for-iOS kit on macOS.
  Not needed for the desktop presets above.

## Clone

```sh
git clone --recurse-submodules https://github.com/iainchesworth/CountdownSolver
```

Already cloned without `--recurse-submodules`? Run:

```sh
git submodule update --init --depth 1 deps/vcpkg
```

## Build and run

```sh
# Configure + build the full project (library, Qt app, tests) with a preset.
# Every preset states its build type explicitly: <toolchain>-debug or
# <toolchain>-release, e.g. windows-msvc-debug, linux-gcc-release,
# linux-clang-debug, macos-clang-release, windows-clang-debug.
cmake --preset windows-msvc-debug -DCMAKE_PREFIX_PATH=<qt>/6.8.3/<arch>
cmake --build --preset windows-msvc-debug

# Run the app.
./build/windows-msvc-debug/bin/countdown_app
```

Just want the solver library and tests, no Qt/GUI?

```sh
cmake --preset linux-gcc-debug -DCOUNTDOWN_BUILD_APP=OFF
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

vcpkg bootstraps itself automatically on first configure — no manual
`vcpkg install` step.

For the full picture — every preset, sanitizer flags, and `cpack`
packaging — see [Building & packaging](building.md). For the test suite
and coverage gate, see [Testing & coverage](testing.md).

## Using your own word list

The letters game ships with a bundled dictionary (~122k words). To use your
own instead, drop a newline-delimited word list at `<config-dir>/words.txt`
(the app reports the exact path per platform in **Settings**) and restart
the app.
