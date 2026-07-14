# Changelog

All notable changes to this project are documented here. Format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); this project
follows [Semantic Versioning](https://semver.org).

`release.yml`'s publish job reads the section matching the pushed tag out
of this file and uses it as the GitHub Release body — keep each entry
human-curated (highlights + known limitations), not a PR/commit dump; the
release still links a "Full Changelog" comparison for the exhaustive list.

## [Unreleased]

## [0.1.0-beta.1] - 2026-07-14

First tagged release, as a **beta**: the language picker translates the
UI, but gameplay (Letters/Numbers/Conundrum rounds, dictionary) is only
fully wired for English so far.

### Added

- C++23 solver engine for Letters, Numbers, and Conundrum rounds,
  generalized beyond a hardcoded 26-letter English alphabet.
- Qt6 desktop app with light/dark/system theming.
- Multilingual UI: language picker covering English, French, German,
  Spanish, Arabic, Hebrew, and Yiddish, with RTL layout support and
  CI-enforced translation completeness.
- Checksummed, Sigstore-attested packages for Windows, macOS, and Linux.

### Known limitations

- Non-English gameplay (rounds, dictionary) isn't wired end-to-end yet —
  only the UI chrome is fully translated. Tracked for the next release.
