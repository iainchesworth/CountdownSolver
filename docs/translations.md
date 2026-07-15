# Translations

The app's UI chrome (menus, buttons, Settings) is translated via Qt
Linguist. This page covers updating an existing translation and adding a
new one — see [How to play](how-to-play.md#languages) for what's
translated today versus what's still English-only.

## How it fits together

Translation source files live at `src/app/translations/countdown_<code>.ts`
(Qt Linguist XML), one per language:

| Code | Language | File |
| --- | --- | --- |
| `fr` | Français | `countdown_fr.ts` |
| `de` | Deutsch | `countdown_de.ts` |
| `es` | Español | `countdown_es.ts` |
| `ar` | العربية | `countdown_ar.ts` |
| `he` | עברית | `countdown_he.ts` |
| `yi` | יידיש | `countdown_yi.ts` |

English has no `.ts` file — it's the literal `qsTr()`/`tr()` source text.
`src/app/CMakeLists.txt`'s `qt_add_translations()` call wires these into
the build: it scans every QML file already registered to the
`countdownsolver_qml` target for translatable strings, plus `solver.cpp`
(listed explicitly, since it lives in the separate `countdownsolver_core`
target). At build time this compiles each `.ts` to a `.qm` and embeds it
as a resource under `:/i18n`; `LanguageManager` (`src/app/language_manager.cpp`)
loads the matching `.qm` when a language is selected.

## Updating an existing translation

1. Regenerate the `.ts` files from current source strings — this is the
   same target CI runs:

   ```sh
   cmake --build --preset <preset> --target update_translations
   ```

   Any new or changed `qsTr()`/`tr()` string shows up as a `<translation
   type="unfinished">` entry (empty, or holding the last-known text) in
   the relevant `.ts` file(s).
2. Open the `.ts` file in **Qt Linguist** (ships with Qt), or edit the
   `<translation>` elements directly, and fill in/review each unfinished
   entry. Linguist clears `type="unfinished"` for you when you mark a
   string reviewed; editing by hand, remove the attribute yourself.
3. Rebuild normally (`cmake --build --preset <preset>`) to recompile the
   `.qm` and pick up your changes.

Before opening a PR, confirm nothing is left unfinished — this is exactly
what CI checks:

```sh
grep -rn 'type="unfinished"' src/app/translations/*.ts
```

Two CI jobs enforce this: **"Check translations are up to date"** reruns
`update_translations` against your changes and fails if that produces any
unfinished entries (catches a string you changed but never re-translated),
and **"Translations complete"** greps the committed `.ts` files as-is
(catches one left `unfinished` by hand). Both need to pass.

### Plural forms

Some strings use Qt's `%n` syntax (e.g. `qsTr("%n letter(s)", "", count)`
in `LettersPage.qml`), which Linguist represents as multiple
`<numerusform>` entries per message — most languages need two (singular/
plural), but Arabic's six grammatical plural categories mean
`countdown_ar.ts` has six `<numerusform>` entries per plural message. Fill
in all of them, not just the first.

### Finding a string

`lupdate` groups each `.ts` file's messages into a `<context><name>`
block named after the QML component it came from (`NumbersPage`,
`LettersPage`, `ConundrumPage`, `SettingsPage`, `Main`) — use that to
jump straight to the right area of a large `.ts` file.

## Adding a new language

Translating the UI chrome is only part of it — a new language needs a few
things wired up before it's selectable at all:

1. Add `translations/countdown_<code>.ts` to the `TS_FILES` list in
   `qt_add_translations()` (`src/app/CMakeLists.txt`), then run
   `update_translations` to generate the initial file and translate it as
   above.
2. Add `{code, "Native name"}` to the `kLanguages` array in
   `src/app/language_manager.cpp`. **This step is easy to miss** — without
   it, `LanguageManager::setLanguage()` rejects the code as unsupported
   and the language never appears in the Settings dropdown, even with a
   fully-translated `.ts`/`.qm`.
3. If the script is right-to-left, no extra code is needed —
   `LanguageManager` derives layout direction from `QLocale(code)`
   automatically. If it needs a font `Theme.qml`'s default (IBM Plex
   Sans) doesn't cover — as Arabic and Hebrew do, via the bundled Noto
   Sans fonts — add it to `Theme.qml`'s `rtlFonts` map and register it in
   `main.cpp`.

That covers UI chrome. **Gameplay is a separate, bigger step** if you're
adding a *new* language beyond the six already wired up (French, German,
Spanish, Arabic, Hebrew, Yiddish — see
[How to play → Languages](how-to-play.md#languages)): it needs a
`letters::Alphabet` mapping in `alphabet_for_language()` and a bundled
dictionary resource in `dictionary_resource_for_language()` (both in
`src/app/solver.cpp`), registered as a Qt resource in `CMakeLists.txt`.
Don't assume a translated UI alone implies a playable round in that
language — check whether both of those are wired up too.
