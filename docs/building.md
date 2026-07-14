# Building & packaging

## Supported platforms

What's actually built and tested by CI — see
[CI & dependencies](ci.md) for the exact matrix:

| Platform | Architecture | Build | Run (packaged build) |
| --- | --- | --- | --- |
| Windows | x64 | MSVC 19.40+ (VS 2022 17.10+ / VS 18), tested on Windows Server 2025 | `.zip` or NSIS installer; Qt runtime is bundled, nothing extra to install |
| macOS | arm64 (Apple Silicon) | AppleClang, tested on macOS 15 (Sequoia) | `.zip` or `.dmg`; Qt runtime is bundled. Intel Macs aren't covered by CI |
| Linux | x64 | GCC 15 or Clang 21, tested on Ubuntu 24.04 | See below — unlike Windows/macOS, Linux packages don't bundle Qt |

`countdown::solver` itself (`-DCOUNTDOWN_BUILD_APP=OFF`) has no GUI or
platform-specific code — see [Architecture](architecture.md) — so it isn't
restricted to this table; any C++23 compiler with CMake 4+ and Ninja
should work even where the full GUI app isn't verified.

**Running Linux packages**: the `.zip`/`.tar.gz` expect a system Qt 6.8+
already installed, since Qt's CMake deploy step doesn't support Linux (see
[Packaging](#packaging) below). The `.deb` declares that dependency
explicitly, so `apt install` refuses on a system that can't satisfy it —
in practice **Debian 13 (trixie) or newer, and Ubuntu 25.04 or newer**.
The `.rpm` resolves its own Qt version requirement automatically via
`rpmbuild`'s auto-requires.

## Presets

Every preset states its build type explicitly:
`<toolchain>-debug` or `<toolchain>-release`, e.g. `windows-msvc-debug`,
`linux-gcc-release`, `linux-clang-debug`, `macos-clang-release`,
`windows-clang-debug`.

```sh
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

Library + tests only, no Qt:

```sh
cmake --preset linux-gcc-debug -DCOUNTDOWN_BUILD_APP=OFF
cmake --build --preset linux-gcc-debug
ctest --preset linux-gcc-debug
```

> Building Qt from source through vcpkg is slow the first time. To iterate
> on just the library and tests, `-DCOUNTDOWN_BUILD_APP=OFF` skips Qt
> entirely.

## Debug builds: sanitizers & hardening

Debug configurations enable, via
[`Sanitizers.cmake`](https://github.com/iainchesworth/CountdownSolver/blob/develop/cmake/Sanitizers.cmake):

- **AddressSanitizer + UndefinedBehaviorSanitizer** on GCC/Clang
  (`-fsanitize=address,undefined -fno-sanitize-recover=all`),
  **AddressSanitizer** on MSVC (`/fsanitize=address`, with the incompatible
  `/RTC` and incremental linking automatically removed);
- **standard-library assertions** (`_GLIBCXX_ASSERTIONS`).

All of this is gated on `$<CONFIG:Debug>`, so Release builds are
unaffected. Turn it off with `-DCOUNTDOWN_ENABLE_SANITIZERS=OFF`.

## Packaging

`cpack` builds a redistributable package from an existing build directory.
The Qt app's runtime, plugins, and QML modules are deployed into the
package automatically (via Qt's `qt_generate_deploy_app_script()` — no
manual `windeployqt`/`macdeployqt` step needed):

```sh
cmake --build --preset windows-msvc-release   # or any other release preset
cd build/windows-msvc-release
cpack                                          # writes CountdownSolver-<ver>-<platform>.zip, etc.
```

A plain `ZIP` archive is always produced; platform-native formats are
added when their packaging tool is available: `NSIS` installer on
Windows, `DragNDrop` (`.dmg`) on macOS, `TGZ` + `DEB` (Debian/Ubuntu) +
`RPM` (Fedora/RHEL and other Red Hat–based distros) on Linux — `DEB`
needs `dpkg-deb`, `RPM` needs `rpmbuild`; each is skipped if its tool
isn't on `PATH`. Linux packages rely on a system Qt6 install — Qt's CMake
deploy step doesn't support Linux.

The `.deb` requires Qt 6.8+ (the app is built against whatever Qt SDK CI
pins — see [ci.md](ci.md) — and its compiled QML metadata hard-requires
that major.minor at runtime). That means **Debian 13 (trixie) or newer,
and Ubuntu 25.04 or newer (including 26.04 LTS)** — Ubuntu 24.04 LTS and
Debian 12 only ship Qt 6.4 and can't satisfy it via `apt install`. The
`.rpm` doesn't need a manual version pin: `rpmbuild`'s auto-requires picks
up Qt's own versioned symbols (`libQt6Core.so.6(Qt_6.8)`) automatically.

Released packages (built via the [release workflow](https://github.com/iainchesworth/CountdownSolver/blob/develop/.github/workflows/release.yml))
also ship a `SHA256SUMS`/`SHA512SUMS` per platform and a
[build provenance attestation](https://docs.github.com/en/actions/security-guides/using-artifact-attestations-to-establish-provenance-for-builds)
for every package — verify a download with:

```sh
sha256sum -c SHA256SUMS
gh attestation verify CountdownSolver-1.2.3-Linux.tar.gz -R iainchesworth/CountdownSolver
```

## Versioning

The project follows [Semantic Versioning](https://semver.org); the
canonical version is `project(... VERSION x.y.z)` in the top-level
`CMakeLists.txt`.

At **build time** (not just configure time) the version and git
provenance — `git describe`, commit hash, branch, and dirty state — are
stamped into a generated `countdown/version.hpp`. Tag releases as
`vX.Y.Z` so `git describe` produces clean version strings.

The information is surfaced two ways:

```console
$ countdownsolver --version
CountdownSolver 0.1.0
  build:  v0.1.0
  commit: 1a2b3c4d...
  branch: main
```

and in the GUI under **Settings → About**.

See [Architecture](architecture.md) for how the source tree is laid out.
