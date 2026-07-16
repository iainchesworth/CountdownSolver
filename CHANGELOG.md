# Changelog

All notable changes to this project are documented here. Format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); this project
follows [Semantic Versioning](https://semver.org).

`release.yml`'s publish job reads the section matching the pushed tag out
of this file verbatim and uses it as the GitHub Release body — write each
entry the way you want it to read as a release: bold-lead bullets
explaining what changed *and why it matters*, grouped under Added/
Fixed/Changed/Known limitations, not a raw PR/commit dump. Include an
`## Artifacts` table by hand if you want one echoed in the release, the
same way v0.1.0-beta.1's entry below does.

## [Unreleased]

### Fixed

- **iOS build configuration no longer locks the app to iPad/landscape-only.**
  `TARGETED_DEVICE_FAMILY` now targets iPhone and iPad, and the
  `Info.plist` orientation lock (plus the `UIRequiresFullScreen` key that
  enforced it) is gone — matching Android's tablet+phone,
  portrait+landscape scope, since the QML shell's layout logic already
  keyed off window size rather than device idiom. Unverified on real
  hardware or a simulator (no Mac available); CI only confirms it still
  compiles.

## [0.2.0-beta.1] - 2026-07-16

Countdown Solver now runs natively on mobile, not just desktop. Tagged as a
**beta** for one specific reason, called out below — everything else here
is release-quality.

### Added

- **Android and iOS mobile support.** The same C++ solver and Qt Quick UI
  that power the desktop app now build and run as native mobile apps.
  Android covers **tablet and phone, portrait and landscape** — a bottom
  tab bar replaces the desktop sidebar on those form factors, with
  layouts purpose-built for each screen size rather than a shrunk-down
  desktop view. iOS is tablet-only and landscape-only for now (see
  *Known limitations*).
- **Bundled IBM Plex Sans/Mono fonts.** The app no longer assumes these
  are system-installed, so type renders identically across every
  platform — this matters most on mobile, where the fonts are never
  preinstalled.
- **Expanded documentation**: an architecture diagram, an FAQ, a
  translation guide, and Android/iOS platform docs, plus issue/PR
  templates and CODEOWNERS for the repo itself.

### Fixed

- RTL solution-step cards (Arabic, Hebrew) no longer sit flush against
  the left edge.
- The backspace glyph (U+232B) has been replaced — it isn't present in
  IBM Plex Sans or Mono at all, so it rendered as a blank box.
- A round of Android/iOS build and packaging bugs found on the first
  real CI runs for those platforms: signed-APK packaging, the release
  workflow's secrets gate, a dropdown indicator rendering as a tofu box
  on Android, and several Xcode/CMake issues specific to cross-compiling
  for iOS.

### Known limitations

- **iOS has not been verified on a real device or simulator.** CI builds
  it (and signs it for release when certificates are configured), but no
  Mac has been available during development to actually run it. iOS also
  stays tablet-only and landscape-only in this release — extending it to
  phone and portrait, the way Android now works, needs someone who can
  verify it on real hardware first.
- One new UI string (the mobile Input/Results toggle) has a Yiddish
  translation that hasn't had a native-speaker review yet.

## Artifacts

Checksummed, with build-provenance attestations:

| Platform | Files |
|---|---|
| Windows | `CountdownSolver-0.2.0-win64.zip` |
| macOS | `CountdownSolver-0.2.0-Darwin.dmg` · `CountdownSolver-0.2.0-Darwin.zip` |
| Linux | `CountdownSolver-0.2.0-Linux.deb` · `.rpm` · `.tar.gz` · `.zip` |
| Android | `CountdownSolver-android-arm64-v8a.apk` · `.aab` |
| Provenance | `sha256sum -c SHA256SUMS`; `gh attestation verify <file> --repo iainchesworth/CountdownSolver` |

## [0.1.0-beta.1] - 2026-07-14

The first tagged release of CountdownSolver, a native Countdown (Letters,
Numbers, Conundrum) solver with a Qt6 desktop app. Tagged as a **beta**
for one specific reason, called out below — everything else here is
release-quality.

### Known limitations

- **Non-English gameplay isn't wired end-to-end yet.** Settings lets you
  switch the display language to **French, German, Spanish, Arabic,
  Hebrew, or Yiddish**, and the UI chrome — menus, buttons, Settings
  itself — translates correctly, including right-to-left layout for
  Arabic and Hebrew. But the Letters/Numbers/Conundrum rounds and the
  solver's dictionary are only fully wired for **English** so far —
  switching languages changes the UI, not yet the game. Closing this gap
  is the target for the next release.

### Added

- **Solver engine.** A C++23 library solving all three Countdown round
  types — Letters, Numbers, Conundrum — generalized beyond a hardcoded
  26-letter English alphabet so other scripts can plug in (the
  groundwork for the multilingual work above).
- **Qt6 desktop app.** Native UI for all three game types, with
  **light**, **dark**, and **system** (follows the OS setting) theming.
- **Multilingual UI infrastructure.** A language picker covering 6
  languages beyond English, RTL layout support, and a CI check that
  fails the build if any UI string is added or changed without a
  finished translation in every language.
- **Security-hardened build pipeline.** CodeQL, dependency-review,
  OSV-Scanner, and OpenSSF Scorecard all gate every PR; every release
  package carries a Sigstore build-provenance attestation.

## Artifacts

Checksummed, with build-provenance attestations:

| Platform | Files |
|---|---|
| Windows | `CountdownSolver-0.1.0-win64.zip` |
| macOS | `CountdownSolver-0.1.0-Darwin.dmg` · `CountdownSolver-0.1.0-Darwin.zip` |
| Linux | `CountdownSolver-0.1.0-Linux.deb` · `.rpm` · `.tar.gz` · `.zip` |
| Provenance | `sha256sum -c SHA256SUMS`; `gh attestation verify <file> --repo iainchesworth/CountdownSolver` |
