# FAQ / troubleshooting

Quick answers, linking out to the full detail rather than repeating it.

**I switched the language in Settings, but the Numbers/Letters/Conundrum
rounds are still in English — is that a bug?**
No — see [How to play → Languages](how-to-play.md#languages). The
language picker currently translates the UI chrome only; gameplay and
the dictionary are English-only for now.

**I unzipped the Linux build and the app won't start / complains about
missing Qt libraries.**
Unlike the Windows/macOS packages, the Linux `.zip`/`.tar.gz` don't
bundle the Qt runtime — you need a system Qt 6.8+ install. See
[Building & packaging → Supported platforms](building.md#supported-platforms)
for exactly what each package format expects.

**`apt install` refuses the `.deb` package.**
The `.deb` requires Qt 6.8+, which only Debian 13+ and Ubuntu 25.04+ ship
via `apt` — see [Building & packaging](building.md#packaging) for why,
and use the `.rpm` or the plain `.tar.gz` (with your own Qt 6.8+ install)
on an older distro instead.

**My custom word list isn't showing up.**
Drop a newline-delimited word list at `<config-dir>/words.txt` (the exact
path is shown in **Settings**) and **restart the app** — it's only read
at startup. See
[Getting started → Using your own word list](getting-started.md#using-your-own-word-list).

**Is there an Android or iOS download?**
Not on the [releases page](https://github.com/iainchesworth/CountdownSolver/releases)
yet as a polished listing, but the release pipeline does produce signed
packages — an Android APK/AAB and an ad-hoc-signed iOS IPA (installable
only on pre-registered devices, not a public App Store build). See
[Building & packaging → Supported platforms](building.md#supported-platforms).
iOS CI is currently broken independent of this — see
[Translations → iOS build currently broken](translations.md#ios-build-currently-broken).

**My PR is failing the "Translations complete" / "Check translations are
up to date" CI check.**
You changed or added a translatable string without updating the `.ts`
files, or left one marked unfinished. See [Translations](translations.md).

**Where do I report a security vulnerability?**
Not as a public issue — see [Security policy](https://github.com/iainchesworth/CountdownSolver/security/policy)
for private reporting.

Didn't find your issue here? [Open one](https://github.com/iainchesworth/CountdownSolver/issues/new/choose) —
see [Contributing](CONTRIBUTING.md) first if it's more than a quick
question.
