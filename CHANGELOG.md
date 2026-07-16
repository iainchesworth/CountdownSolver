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

- **The iOS install link in release notes was never actually clickable.**
  GitHub's release-notes markdown renderer sanitizes link `href`s to a
  fixed scheme allowlist and silently strips any `<a>` using an
  unrecognized scheme like `itms-services://`, leaving plain unclickable
  text — confirmed by rendering `v0.3.0-beta.2`'s own release body through
  GitHub's markdown API. Release notes now link to a small
  `docs/install-ios.html` page (hosted on GitHub Pages, not subject to
  that sanitizer) that builds the real `itms-services://` link
  client-side from a `?tag=` query parameter instead.

## [0.3.0-beta.2] - 2026-07-16

No functional or build changes from v0.3.0-beta.1 — this release exists
solely to ship working install links for the mobile builds that release
already signed. Tagged as a **beta** to match v0.3.0-beta.1's own
still-open item, called out below.

### Added

- **Working install links, straight from the GitHub Release.** `ios-release`
  now also generates `manifest.plist`, the property list Safari's
  `itms-services://` OTA-install trigger requires (a bare `.ipa` link can't
  be installed directly, unlike a sideloaded Android APK). `publish` then
  appends an `## Install` section to each release body with a direct
  Android APK download link and an `itms-services://` iOS install link —
  tappable straight from a phone's browser on the Release page, no manual
  URL construction needed. The iOS link only ever works on a device whose
  UDID is already registered in the signing provisioning profile.
- **A new [Installing](https://iainchesworth.github.io/CountdownSolver/installing/)
  docs page**, covering both mobile install flows end-to-end: the Android
  "install unknown apps" permission, the iOS ad-hoc/UDID-registration
  constraint, the OTA-manifest mechanics, and how to verify a download's
  checksums/attestations.

### Known limitations

- **iOS has not been verified on a real device or simulator.** Unchanged
  from v0.3.0-beta.1 — no Mac has been available during development to
  actually run it.

## Artifacts

Checksummed, with build-provenance attestations and cosign-signed
manifests:

| Platform | Files |
|---|---|
| Windows | `CountdownSolver-0.3.0-win64.zip` |
| macOS | `CountdownSolver-0.3.0-Darwin.dmg` · `CountdownSolver-0.3.0-Darwin.zip` |
| Linux | `CountdownSolver-0.3.0-Linux.deb` · `.rpm` · `.tar.gz` · `.zip` |
| Android | `CountdownSolver-android-arm64-v8a.apk` · `.aab` |
| iOS | `CountdownSolver-ios-adhoc.ipa` (ad-hoc signed; sideload only) · `manifest.plist` (OTA install manifest) |
| Provenance | `sha256sum -c SHA256SUMS`; `gh attestation verify <file> --repo iainchesworth/CountdownSolver`; `cosign verify-blob --bundle SHA256SUMS.sigstore.json` |

## [0.3.0-beta.1] - 2026-07-16

iOS now matches Android's device coverage, and the release pipeline picked
up a round of supply-chain hardening. Tagged as a **beta** for one
specific reason, called out below — everything else here is
release-quality.

### Added

- **Fuzzing coverage for the solver.** Three libFuzzer harnesses (UTF-8
  decoding, the letters game's rack validation/solving, the numbers game's
  solver) now run under ClusterFuzzLite on every PR, daily as a batch job
  building a corpus, and with mandatory corpus pruning — closing the gap
  that had OpenSSF Scorecard's Fuzzing check failing (it only recognises
  OSS-Fuzz, ClusterFuzzLite, or language-specific property-testing tools,
  none of which C++ had here before).
- **Cosign-signed release manifests.** The combined SHA256SUMS/SHA512SUMS
  checksum manifests are now signed with `cosign sign-blob` (keyless, via
  the same GitHub OIDC identity already used for build provenance) and the
  `.sigstore.json` bundles are uploaded alongside the release — Sigstore
  attestations alone aren't visible to Scorecard's Signed-Releases check,
  since it only looks at downloadable release assets.
- **A committed 14-day initial-response window for vulnerability reports**,
  replacing the previous "no SLA" wording in `SECURITY.md` — required for
  OpenSSF's Best Practices ("CII") Passing tier.
- **A signed iOS release package.** With signing certificates now
  configured, `release.yml` produces an ad-hoc signed IPA
  (`CountdownSolver-ios-adhoc.ipa`) for this release — installable on
  devices registered in the provisioning profile, not yet an App Store
  build.

### Fixed

- **iOS now targets iPhone and all orientations**, matching Android's
  tablet+phone, portrait+landscape scope. `TARGETED_DEVICE_FAMILY` was
  locked to iPad-only and `Info.plist` locked orientation to landscape
  (plus `UIRequiresFullScreen` to stop Split View from bypassing that
  lock) — leftover config from the original tablet-only scope; the QML
  shell's layout logic already keyed off window size, not device idiom,
  so nothing else was blocking it. Unverified on real hardware or a
  simulator (no Mac available); CI only confirms it still compiles.
- iOS code signing is now scoped to the app target instead of applying
  project-wide.
- The iOS `ExportOptions.plist` now maps `provisioningProfiles` for the
  app target explicitly, instead of relying on automatic matching.
- Release-signing secrets are now passed through `env:` in
  `release.yml` rather than interpolated directly into a `run:` block,
  closing a code-injection-via-template-expansion risk flagged by
  `zizmor`.

### Known limitations

- **iOS has not been verified on a real device or simulator.** CI builds
  it (and signs it for release when certificates are configured), but no
  Mac has been available during development to actually run it.
- One UI string (the mobile Input/Results toggle, added last release) has
  a Yiddish translation that hasn't had a native-speaker review yet.

## Artifacts

Checksummed, with build-provenance attestations and cosign-signed
manifests:

| Platform | Files |
|---|---|
| Windows | `CountdownSolver-0.3.0-win64.zip` |
| macOS | `CountdownSolver-0.3.0-Darwin.dmg` · `CountdownSolver-0.3.0-Darwin.zip` |
| Linux | `CountdownSolver-0.3.0-Linux.deb` · `.rpm` · `.tar.gz` · `.zip` |
| Android | `CountdownSolver-android-arm64-v8a.apk` · `.aab` |
| iOS | `CountdownSolver-ios-adhoc.ipa` (ad-hoc signed; sideload only) |
| Provenance | `sha256sum -c SHA256SUMS`; `gh attestation verify <file> --repo iainchesworth/CountdownSolver`; `cosign verify-blob --bundle SHA256SUMS.sigstore.json` |

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
