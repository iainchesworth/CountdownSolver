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
| `APPLE_PROVISIONING_PROFILE_NAME` | The provisioning profile's own display name in Apple's system, exactly as named there (passed to Xcode as `PROVISIONING_PROFILE_SPECIFIER`) — a secret rather than a repo constant, since it has to match whatever you actually name the profile, not a name this repo can predict |

If any of these are unset, the corresponding release job fails fast with an
explicit error rather than silently producing an unsigned package.

**iOS distribution note**: a standard Apple Developer account can only
produce an *ad-hoc* signed IPA — installable solely on devices whose UDIDs
are pre-registered in the provisioning profile above, not on arbitrary iOS
devices the way a sideloaded Android APK is. Wider distribution (TestFlight
or an Apple Developer Enterprise account) is a separate, later decision.

Alongside the IPA, `ios-release` also generates `manifest.plist` — the
property list Safari's `itms-services://` OTA-install trigger needs (a
bare `.ipa` link can't be installed directly, unlike an Android APK).
`release.yml`'s `publish` job then appends working install links to the
GitHub Release body itself: a direct Android APK link, and for iOS a link
to [`install-ios.html`](install-ios.html) (GitHub Pages) rather than a raw
`itms-services://` URL — GitHub's release-notes markdown renderer strips
`<a>` tags using non-allowlisted schemes like `itms-services://` entirely,
so that page builds the real install link client-side instead. See
[Installing](installing.md) for the end-user install flow.

**Current verification status** (no Android SDK/NDK, no Mac/Xcode
available in this project's day-to-day dev environment — everything below
was confirmed via real CI runs, not locally):

- **Android is verified end-to-end, including a real intermittent signing
  bug caught and fixed**: `android-release` produces a real signed
  APK/AAB, installed and run on a real tablet emulator (Pixel Tablet
  profile, API 34) — confirming fonts, backspace icon, and RTL layout
  fixes render correctly on the actual device, not just "the build
  succeeded." That same verification also caught a real defect CI's
  file-existence/checksum checks had been silently missing: `android-build/`
  contains both Qt's own automatic unsigned APK (produced during the plain
  "Build" step) and this job's explicitly-signed one side by side, and a
  bare `find ... -name "*.apk"` glob matched both — since `-exec cp` has no
  defined match order, it could silently overwrite the signed APK with the
  unsigned one depending on filesystem traversal order. One real release
  run produced a genuinely unsigned artifact this way (`apksigner verify`:
  "Missing META-INF/MANIFEST.MF") while every existing CI check reported
  success. Fixed by matching only `*-signed.apk`, plus an explicit
  `apksigner verify` step that now fails the job outright if the final
  artifact isn't actually signed - so this class of bug can't silently
  ship again.
- **Extended to phone and portrait, verified the same way**: the signed
  APK from the actual `v0.2.0-beta.1` tagged release was installed on that
  same emulator and driven through all four form factors (tablet
  landscape/portrait, phone landscape/portrait) via `adb shell wm size`/
  `settings put system user_rotation` overrides — each one renders
  correctly, not just the tablet-landscape shape the app originally
  shipped with.
- **`ios-build` (unsigned compile) is verified**: it's passed real Xcode
  CI runs repeatedly since the fixes below landed.
- **`ios-release` (signed, ad-hoc) has never run for real** — blocked on
  you creating the five `APPLE_*`/`IOS_*` secrets above (pending Apple
  Developer Program enrollment). Treat the first real run's failures as
  expected things to fix (exact Xcode archive/export mechanics, whether
  the provisioning-profile-specifier matches), not as a sign the approach
  is wrong.

### iOS build fixes (all confirmed via real Xcode CI runs)

**Duplicate-translations-output at the CMake Generate step.** Xcode's "new
build system" refused to generate the project because two targets —
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
shared output; Xcode's new build system doesn't. Fixed in
`src/app/CMakeLists.txt` by moving the `qt_add_translations()` call to
after `qt_add_executable()` and passing `IMMEDIATE_CALL`, so it runs
synchronously in `src/app`'s own scope instead of deferring to the root.

**`find_package(Qt6)` itself failing** with `Parameters to $<AND> must
resolve to either '0' or '1'`. Around 30 plugin targets (image format
plugins, platform integration plugins, qmltooling debug/profiler plugins,
and the Apple-only permission/TLS/reachability plugins) don't export
`QT_DEFAULT_PLUGIN` at all on the Qt 6.8.3-for-iOS kit — Qt's own
plugin-linkage genex reads that property unwrapped as a `$<AND>` operand,
and an unset property is an empty string, not `0`/`1`. Confirmed identical
on both CMake 4.3.3 and 4.0.3, ruling out a CMake-version-specific cause;
no matching upstream Qt bug report found. Worked around in
`src/app/CMakeLists.txt` (`if(IOS)` block right after `find_package`) by
explicitly setting the property on each affected plugin target, using the
desktop Qt 6.8.3 kit's own correct values as the reference.

**Compilation failing** with `'to_chars' is unavailable: introduced in iOS
16.3`. `std::format` (used by libc++) pulls in `std::to_chars`, gated by
iOS SDK availability annotations against whatever deployment target is
set — none was set at all before. Fixed by adding
`CMAKE_OSX_DEPLOYMENT_TARGET=16.3` to the `ios-base` preset in
`CMakePresets.json`.
