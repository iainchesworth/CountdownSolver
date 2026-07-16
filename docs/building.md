# Building & packaging

## Supported platforms

What's actually built and tested by CI — see
[CI & dependencies](ci.md) for the exact matrix:

| Platform | Architecture | Build | Run (packaged build) |
| --- | --- | --- | --- |
| Windows | x64 | MSVC 19.40+ (VS 2022 17.10+ / VS 18), tested on Windows Server 2025 | `.zip` or NSIS installer; Qt runtime is bundled, nothing extra to install |
| macOS | arm64 (Apple Silicon) | AppleClang, tested on macOS 15 (Sequoia) | `.zip` or `.dmg`; Qt runtime is bundled. Intel Macs aren't covered by CI |
| Linux | x64 | GCC 15 or Clang 21, tested on Ubuntu 24.04 | See below — unlike Windows/macOS, Linux packages don't bundle Qt |
| Android | arm64-v8a | CI-verified via the `android-arm64-v8a-debug` preset (Qt's Android kit). Tablet and phone, portrait and landscape | Signed APK/AAB via `release.yml`'s `android-release` job (needs maintainer-supplied keystore secrets — see [CI & dependencies](ci.md#signed-mobile-release-packaging)); proven end-to-end against a real tagged release (`v0.2.0-beta.1`), installed and driven on a real emulator across all four form factors. See [Installing](installing.md#android) |
| iOS | device (arm64) | CI-verified via the `ios-debug` preset (Qt's iOS kit), unsigned. Targets iPhone and iPad, all orientations — same breadth as Android, but unverified: no Mac available to run it on real hardware or a simulator | Ad-hoc signed IPA via `release.yml`'s `ios-release` job — installable only on devices pre-registered in the provisioning profile, not a public App Store build. Signing has never run for real yet (no Apple Developer secrets configured) and nothing has been verified on real hardware — no Mac available in day-to-day dev. See [Installing](installing.md#ios-ad-hoc-pre-registered-devices-only) |

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

**Android/iOS**: CPack packaging is skipped entirely for these targets
([`Packaging.cmake`](https://github.com/iainchesworth/CountdownSolver/blob/develop/cmake/Packaging.cmake)) —
mobile distribution goes through `androiddeployqt` and an Xcode
archive+export instead ([CI & dependencies](ci.md#signed-mobile-release-packaging)
has the full signing setup). The debug builds CI runs on every push are
build-only and unsigned, purely to catch breaks early; a real signed
package only comes out of a tagged release (Android only, for now — see
above). `ios-build` compiles cleanly in CI; see
[CI & dependencies → iOS build fixes](ci.md#ios-build-fixes-all-confirmed-via-real-xcode-ci-runs)
for the Xcode-specific issues that were fixed to get there.

## Presets

Every preset states its build type explicitly:
`<toolchain>-debug` or `<toolchain>-release`, e.g. `windows-msvc-debug`,
`linux-gcc-release`, `linux-clang-debug`, `macos-clang-release`,
`windows-clang-debug`, `android-arm64-v8a-debug`,
`android-arm64-v8a-release`, `ios-debug`, `ios-release`.

```sh
cmake --preset windows-msvc-debug
cmake --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug
```

### Mobile builds (Android/iOS)

Android and iOS both cover tablet and phone, portrait and landscape — the
QML shell switches between a desktop-style sidebar and a bottom-tab-bar
layout based on window size, not the OS or device idiom. iOS's build
configuration now matches Android's scope, but — unlike Android — it's
unverified: no Mac is available to actually run it on a simulator or real
device and confirm the layouts render correctly. These presets set
`COUNTDOWN_BUILD_TESTS=OFF`: there's no
way to run a CTest-launched native test executable on Android/iOS without
an emulator/simulator/on-device runner, so `ctest` isn't part of the
mobile flow.

```sh
# Android (arm64-v8a) - needs a Qt-for-Android kit (QT_ANDROID_ROOT below)
# and a desktop Qt kit for cross-build host tools (QT_HOST_PATH).
QT_HOST_PATH=<desktop-qt-root> QT_ANDROID_ROOT=<qt-android-root> \
  cmake --preset android-arm64-v8a-debug -DCMAKE_PREFIX_PATH=<qt-android-root>
cmake --build --preset android-arm64-v8a-debug

# iOS (device) - macOS + Xcode + a Qt-for-iOS kit only; unsigned by default.
cmake --preset ios-debug -DCMAKE_PREFIX_PATH=<qt-ios-root>
cmake --build --preset ios-debug
```

`cpack` doesn't apply to either target — `cmake/Packaging.cmake` skips CPack
entirely for `ANDROID`/`IOS` configures. Real packaging is
[Qt's `androiddeployqt`](https://doc.qt.io/qt-6/androiddeployqt.html)
(APK/AAB) and Xcode archive + `xcodebuild -exportArchive` (IPA),
respectively — see the `android-release`/`ios-release` jobs in the
[release workflow](https://github.com/iainchesworth/CountdownSolver/blob/develop/.github/workflows/release.yml)
for the full signed pipeline, and [CI & dependencies](ci.md) for the
required signing secrets.

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
CountdownSolver 0.2.0
  build:  v0.2.0-beta.1
  commit: 1a2b3c4d...
  branch: main
```

and in the GUI under **Settings → About**.

See [Architecture](architecture.md) for how the source tree is laid out.
