# FAQ / troubleshooting

Quick answers, linking out to the full detail rather than repeating it.

**I switched the language in Settings — does that change gameplay too, or
just the menus?**
Both. Letters and Conundrum solve against that language's own dictionary
and alphabet rules, not just translated labels — see
[How to play → Languages](how-to-play.md#languages). Numbers is
language-independent (it's arithmetic).

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
Android, yes — every [release](https://github.com/iainchesworth/CountdownSolver/releases)
from `v0.2.0-beta.1` on ships a signed APK/AAB, covering tablet and phone
in both portrait and landscape. iOS builds successfully in CI but isn't
signed for release yet (needs a maintainer with an Apple Developer account
to configure signing secrets — see
[CI & dependencies → Signed mobile release packaging](ci.md#signed-mobile-release-packaging)),
and stays tablet-only/landscape-only until someone can verify phone and
portrait layouts on real hardware. See
[Building & packaging → Supported platforms](building.md#supported-platforms).

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
