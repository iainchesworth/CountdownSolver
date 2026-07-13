# CountdownSolver

A cross-platform solver for the two games from the TV show **Countdown**:

- **Numbers** — given six numbers and a target (100–999), find an arithmetic
  expression (using `+ - x /`, each number at most once, only positive whole
  intermediate results, exact division) that reaches or gets closest to the
  target.
- **Letters** — given a set of letters, find the longest dictionary word(s) that
  can be spelled from them.

All game logic lives in a GUI-free library (`countdown::solver`); a thin **Qt 6
Quick (QML)** application provides the GUI, wired to the library through a small
`Solver` backend (`src/app/solver.hpp`). The QML/visual design lives in
`src/app/qml/` (see `src/app/DESIGN_SPEC.md`).

## Design highlights

This project deliberately uses modern **C++23** throughout:

| Feature | Where |
| --- | --- |
| `std::expected` + monadic `and_then` / no exceptions | [`error.hpp`](src/lib/include/countdown/error.hpp), [`numbers_game.cpp`](src/lib/src/numbers/numbers_game.cpp) |
| Deducing `this` fluent builders | [`numbers_game.hpp`](src/lib/include/countdown/numbers/numbers_game.hpp), [`letters_game.hpp`](src/lib/include/countdown/letters/letters_game.hpp) |
| `if consteval` compile-time vs runtime branching | [`frequencies.hpp`](src/lib/include/countdown/letters/frequencies.hpp) |
| `views::zip` / `stride` / `enumerate` / `transform` + `ranges::to` | [`dictionary.cpp`](src/lib/src/letters/dictionary.cpp), [`solution.cpp`](src/lib/src/numbers/solution.cpp) |
| Platform code selected by CMake, **no `#ifdef`** | [`src/app/platform/`](src/app/platform/) |
| Build-time git version stamp | [`GenerateVersion.cmake`](cmake/GenerateVersion.cmake), [`version.hpp.in`](src/lib/include/countdown/version.hpp.in) |

## Requirements

- A **C++23** compiler: GCC 14+, Clang 18+, or MSVC 19.40+ (VS 2022 17.10+ / VS 18).
- **CMake 4.0+** and **Ninja**.
- **Git** — **vcpkg is vendored** as a submodule at [`deps/vcpkg`](deps/vcpkg),
  so no machine-wide vcpkg or `VCPKG_ROOT` is required. vcpkg provides **Catch2**
  (declared in [`vcpkg.json`](vcpkg.json), installed automatically in manifest
  mode; baseline pinned to the submodule commit).
- **Qt 6.8** — used as an official **prebuilt** package, not built from source
  (Qt's deep build tree overruns Windows' `MAX_PATH`, and a source build is
  huge). Install it with [`ci/install-qt.ps1`](ci/install-qt.ps1) /
  [`ci/install-qt.sh`](ci/install-qt.sh) (both use `aqtinstall`), then point the
  build at it with `-DCMAKE_PREFIX_PATH=<qt>/6.8.3/<arch>`. CI does the same via
  `install-qt-action`. See [`ci/README.md`](ci/README.md). Only needed when
  building the app (`COUNTDOWN_BUILD_APP=ON`, the default).

Clone with submodules (or initialise them after cloning):

```sh
git clone --recurse-submodules https://github.com/iainchesworth/CountdownSolver
# or, in an existing clone:
git submodule update --init --depth 1 deps/vcpkg
```

The vcpkg CMake toolchain bootstraps the vendored vcpkg automatically on the
first configure, and CMake will initialise the submodule for you if it is
missing.

> **Note:** building Qt from source through vcpkg is slow the first time. To
> iterate on just the library and tests, configure with `-DCOUNTDOWN_BUILD_APP=OFF`,
> which skips Qt entirely.

## Branching model

- **`main`** — release branch. Only tagged releases are merged here.
- **`develop`** — the integration branch; all day-to-day work targets `develop`.
- Feature branches branch off `develop` and merge back via pull request.

## Building

```sh
# Configure + build the full project (library, Qt app, tests) with a preset.
cmake --preset windows-msvc      # or linux-gcc / linux-clang / macos-clang / windows-clang
cmake --build --preset windows-msvc

# Run the test suite via CTest.
ctest --preset windows-msvc
```

Library + tests only (no Qt):

```sh
cmake --preset linux-gcc -DCOUNTDOWN_BUILD_APP=OFF
cmake --build --preset linux-gcc
ctest --preset linux-gcc
```

## Debug builds: sanitizers & hardening

Debug configurations enable, via [`Sanitizers.cmake`](cmake/Sanitizers.cmake):

- **AddressSanitizer + UndefinedBehaviorSanitizer** on GCC/Clang
  (`-fsanitize=address,undefined -fno-sanitize-recover=all`), **AddressSanitizer**
  on MSVC (`/fsanitize=address`, with the incompatible `/RTC` and incremental
  linking automatically removed);
- **standard-library assertions** (`_GLIBCXX_ASSERTIONS`).

All of this is gated on `$<CONFIG:Debug>`, so Release builds are unaffected.
Turn it off with `-DCOUNTDOWN_ENABLE_SANITIZERS=OFF`.

## Packaging

`cpack` builds a redistributable package from an existing build directory. The
Qt app's Qt runtime, plugins, and QML modules are deployed into the package
automatically (via Qt's `qt_generate_deploy_app_script()` — no manual
`windeployqt`/`macdeployqt` step needed):

```sh
cmake --build --preset windows-msvc-release   # or any other release preset
cd build/windows-msvc-release
cpack                                          # writes CountdownSolver-<ver>-<platform>.zip, etc.
```

A plain `ZIP` archive is always produced; a platform-native format is added
when its packaging tool is available: `NSIS` installer on Windows, `DragNDrop`
(`.dmg`) on macOS, `TGZ` + `DEB` on Linux. Linux packages rely on a system Qt6
install — Qt's CMake deploy step doesn't support Linux.

## Versioning

The project follows **[Semantic Versioning](https://semver.org)**; the canonical
version is `project(... VERSION x.y.z)` in the top-level `CMakeLists.txt`.

At **build time** (not just configure time) the version and git provenance —
`git describe`, commit hash, branch, and dirty state — are stamped into a
generated `countdown/version.hpp`. Tag releases as `vX.Y.Z` so `git describe`
produces clean version strings.

The information is surfaced two ways:

```console
$ countdown_app --version
CountdownSolver 0.1.0
  build:  v0.1.0
  commit: 1a2b3c4d...
  branch: main
```

and in the GUI under **Settings → About**.

## Testing (TDD)

Testing is mandatory and drives development:

- **Unit tests** ([`tests/unit`](tests/unit)) cover each behaviour in isolation,
  including the Numbers, Letters, and Conundrum game logic in `countdown::solver`.
- **Integration tests** ([`tests/integration`](tests/integration)) exercise the
  file-backed dictionary and full solve pipelines together.
- **App tests** ([`tests/app`](tests/app)) exercise the Qt-facing `Solver`
  facade with Qt Test.
- **QML tests** ([`tests/qml`](tests/qml)) drive the real `Countdown` QML
  module (Numbers/Letters/Conundrum/Settings pages) headlessly with Qt Quick
  Test, exercising the QML → `solver` → library round trip end to end.

The Catch2 suites are registered with CTest via `catch_discover_tests`, so each
`TEST_CASE` is an individually runnable CTest; the Qt Test/Quick Test suites
each register as a single CTest entry covering every case in that binary.

### Coverage

A dedicated `linux-gcc-coverage` preset builds everything with `--coverage`
(gcov) instrumentation:

```console
$ cmake --preset linux-gcc-coverage -DCMAKE_PREFIX_PATH=<qt>/6.8.3/gcc_64
$ cmake --build --preset linux-gcc-coverage
$ QT_QPA_PLATFORM=offscreen ctest --preset linux-gcc-coverage
$ gcovr --root . --filter 'src/.*' --exclude 'src/app/main\.cpp' \
    --exclude-throw-branches --exclude-unreachable-branches \
    --gcov-ignore-errors=no_working_dir_found \
    --object-directory build/linux-gcc-coverage --print-summary
```

CI runs this on every push/PR (`Coverage (Linux, GCC)` in `ci.yml`) and fails
the job below 80% line or branch coverage.

The **letters** game ships with a bundled default dictionary (~122k words). Drop
a custom newline-delimited word list at `<config-dir>/words.txt` (the app
reports the location per platform) to solve with your own list instead.

## Layout

```
src/lib/     countdown::solver — all game logic (no GUI, no platform code)
src/app/     Qt 6 Quick (QML) GUI: solver.* backend + qml/ design
src/app/platform/{windows,macos,linux}/   one impl each, chosen by CMake
tests/unit/          unit tests
tests/integration/   integration tests
cmake/               shared CMake modules (strict warnings-as-errors)
deps/vcpkg/          vendored vcpkg (git submodule, pinned)
```

## License

GPL-3.0-or-later. See [LICENSE](LICENSE).
