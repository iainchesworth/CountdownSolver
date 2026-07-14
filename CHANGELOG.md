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
