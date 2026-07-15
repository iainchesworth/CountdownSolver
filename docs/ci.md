# CI & dependency provisioning

Qt is **not** built from source (it is huge, and Qt's deeply-nested build
tree blows past Windows' 260-char `MAX_PATH` under a normal project path).
Instead we use the **official prebuilt Qt binaries** via
[`aqtinstall`](https://github.com/miurahr/aqtinstall):

- **CI** uses `jurplel/install-qt-action` (a wrapper around aqtinstall) —
  see [`.github/workflows/ci.yml`](https://github.com/iainchesworth/CountdownSolver/blob/develop/.github/workflows/ci.yml).
- **Locally**, run the matching script below; both install the same
  pinned Qt and print the `CMAKE_PREFIX_PATH` to pass to CMake.

vcpkg still provides Catch2 (the `tests` feature); it no longer provides
Qt.

## Pinned version

`QT_VERSION = 6.8.3` (keep this in sync across the scripts here and the CI
workflow's `install-qt-action` `version:` input).

## Local install

```powershell
# Windows (PowerShell)
pwsh ci/install-qt.ps1                 # installs to C:\Qt by default
```

```bash
# Linux / macOS
./ci/install-qt.sh                     # installs to ~/Qt by default
```

Then configure the app build against it, e.g. on Windows:

```powershell
cmake --preset windows-msvc-debug -DCOUNTDOWN_BUILD_APP=ON `
  -DCMAKE_PREFIX_PATH=C:/Qt/6.8.3/msvc2022_64
cmake --build --preset windows-msvc-debug
```

`CMAKE_PREFIX_PATH` per platform:

| Platform | Path |
| --- | --- |
| Windows (MSVC) | `<outdir>/6.8.3/msvc2022_64` |
| Linux (GCC)    | `<outdir>/6.8.3/gcc_64` |
| macOS          | `<outdir>/6.8.3/macos` |
| Android (arm64-v8a) | `<outdir>/6.8.3/android_arm64_v8a` — also needs a **second**, desktop Qt install (`QT_HOST_PATH`) for cross-build host tools (`moc`, `qmlcachegen`, ...) |
| iOS (device)   | `<outdir>/6.8.3/ios` — macOS/Xcode only |

## Mobile CI jobs

Two build-only, unsigned jobs in [`ci.yml`](https://github.com/iainchesworth/CountdownSolver/blob/develop/.github/workflows/ci.yml)
gate every PR/branch push, mirroring the desktop `build-and-test` jobs but
without a `ctest` step (mobile presets set `COUNTDOWN_BUILD_TESTS=OFF` —
see [Building & packaging](building.md)):

- **`android-build`** (`ubuntu-24.04`): installs a JDK, the Android SDK, a
  pinned NDK version, and *two* Qt kits via `install-qt-action` — a desktop
  one (`QT_HOST_PATH`) and an Android one (`QT_ANDROID_ROOT`) — then
  configures/builds the `android-arm64-v8a-debug` preset.
- **`ios-build`** (`macos-15`, ships Xcode already): installs a single
  Qt-for-iOS kit, then configures/builds the `ios-debug` preset (unsigned,
  per that preset's `CODE_SIGNING_ALLOWED=NO` cache variables).

## Signed mobile release packaging

The [release workflow](https://github.com/iainchesworth/CountdownSolver/blob/develop/.github/workflows/release.yml)'s
`android-release`/`ios-release` jobs produce a real, signed, distributable
package for each mobile platform (alongside the existing signed-by-checksum
Windows/Linux/macOS packages, all with the same SHA256/512 + Sigstore
attestation treatment). This needs the following **GitHub Secrets**
(Settings → Secrets and variables → Actions), which you create yourself —
the workflow never sees or stores the underlying key material beyond the
lifetime of a single job run:

| Secret | Purpose |
| --- | --- |
| `ANDROID_KEYSTORE_BASE64` | Your Android release keystore file, base64-encoded (`base64 -w0 release.keystore`) |
| `ANDROID_KEYSTORE_PASSWORD` | The keystore's store password |
| `ANDROID_KEY_ALIAS` | The signing key's alias inside the keystore |
| `ANDROID_KEY_PASSWORD` | The signing key's own password |
| `IOS_DIST_CERTIFICATE_BASE64` | Your Apple Distribution certificate `.p12`, base64-encoded |
| `IOS_DIST_CERTIFICATE_PASSWORD` | The `.p12`'s export password |
| `IOS_PROVISIONING_PROFILE_BASE64` | An **ad-hoc** provisioning profile (`.mobileprovision`), base64-encoded |
| `APPLE_TEAM_ID` | Your Apple Developer Team ID |

If any of these are unset, the corresponding release job fails fast with an
explicit error rather than silently producing an unsigned package.

**iOS distribution note**: a standard Apple Developer account can only
produce an *ad-hoc* signed IPA — installable solely on devices whose UDIDs
are pre-registered in the provisioning profile above, not on arbitrary iOS
devices the way a sideloaded Android APK is. Wider distribution (TestFlight
or an Apple Developer Enterprise account) is a separate, later decision.

**Android/iOS signing mechanics are unverified locally** (no Android
SDK/NDK, no Mac/Xcode available in this project's day-to-day dev
environment) — treat a first real release run's failures in these two
jobs as expected things to fix (exact NDK version, `androiddeployqt`'s CLI
surface, the generated Xcode project/scheme names), not as a sign the
overall approach is wrong.

**`ios-build` previously failed at the CMake Generate step** with Xcode's
"new build system" refusing to generate the project because two targets —
`CountdownSolver_lrelease` and an internally-created
`countdownsolver_qml_countdownsolver_qml_translations` — both independently
referenced the same `countdown_<lang>.qm` output with no dependency between
them. Root-caused (by reading Qt's actual CMake macro source,
`Qt6CoreMacros.cmake`'s `_qt_internal_process_resource()`) to
`qt_add_translations()` self-deferring to run at the end of
`PROJECT_SOURCE_DIR`'s scope by default: when it actually ran, its
`CMAKE_CURRENT_SOURCE_DIR` was the repo root, not `src/app`, which didn't
match `countdownsolver_qml`'s own `SOURCE_DIR` — a mismatch that macro
treats as "attaching this resource from a different directory than the
target lives in," causing it to defensively wrap the embedding in that
extra target instead of attaching directly. Ninja tolerates the resulting
shared output; Xcode's new build system doesn't.

Fixed in `src/app/CMakeLists.txt` by moving the `qt_add_translations()` call
to after `qt_add_executable()` and passing `IMMEDIATE_CALL`, so it runs
synchronously in `src/app`'s own scope instead of deferring to the root —
matching `countdownsolver_qml`'s `SOURCE_DIR` and avoiding the wrapper
target entirely. Confirmed empirically on this machine: the wrapper target
appeared 42 times in a Windows/Ninja `build.ninja` before this fix and 0
times after, with all 70 tests still passing (this doesn't need Xcode to
verify, since the underlying CMake target graph is generator-independent).
Still **unverified against a real Xcode Generate step** — no Mac available
in this project's day-to-day dev environment — so treat the first real
`ios-build` CI run against this fix as the actual test.
