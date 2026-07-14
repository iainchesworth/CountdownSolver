# How to play

CountdownSolver has one screen per round, picked from the sidebar, plus a
Settings screen. This page walks through each one: what to enter and how
to read the result.

## Numbers

![Numbers page](assets/screenshots/numbers.png)

1. Fill the six selection tiles from the number pads: the accent-coloured
   pad (25, 50, 75, 100) for the "large" numbers, and 1–10 for the
   "small" ones — pick any mix, same as on the show.
2. Enter a target between 100 and 999 on the keypad.
3. The result card shows:
   - The best value the solver reached, in large type.
   - A badge next to it — filled accent if it's an **exact** match, or
     "N away" if not (only shown when *flag inexact* is on in Settings;
     otherwise it's shown quietly either way).
   - Numbered **working steps**, one calculation per line (e.g.
     `75 × 6 = 450`), building up to the final value. Each number from the
     six tiles is used at most once, and every intermediate result stays a
     positive whole number — same constraints as the TV show.

If a tile or the target is still empty, the card prompts you for what's
missing instead of showing a result.

## Letters

![Letters page](assets/screenshots/letters.png)

1. Fill the nine letter tiles using the on-screen keyboard (or your
   keyboard) — any mix of vowels and consonants, same as the show.
2. The result card shows:
   - The **longest word(s)** found, as accent-coloured chips — there can
     be more than one if several words tie for longest.
   - Every other valid word, grouped by length (longest first) as outline
     chips, with a "showing X of Y" count if the list is capped (see
     *max results* in Settings below).

## Conundrum

![Conundrum page](assets/screenshots/conundrum.png)

1. Nine scrambled letters are shown as tiles — use the `↻` button to
   generate a new one, or type your own nine letters.
2. Reveal the solution: a 3-column grid of accent tiles spells out the
   single word that uses all nine letters. Any other valid nine-letter
   answers are listed below it.

## Settings

![Settings page](assets/screenshots/settings.png)

- **Appearance** — light, dark, or system (follows your OS setting).
- **Solver** — minimum word length for Letters results, whether to flag
  inexact Numbers results with the "N away" badge, and a max-results
  slider to cap how many Letters words are shown.
- **Dictionary** — switch between the bundled sample word list and the
  full ~122k-word dictionary. To use your own word list entirely, see
  [Using your own word list](getting-started.md#using-your-own-word-list).

## Languages

Settings also has a language picker: **English**, plus **French, German,
Spanish, Arabic, Hebrew, and Yiddish** — including right-to-left layout
for Arabic and Hebrew, shown here with the display language set to
Arabic:

![Settings page in Arabic, showing the mirrored right-to-left layout](assets/screenshots/settings-arabic-rtl.png)

As of `0.1.0-beta.1`, this switches the app's UI chrome — menus, buttons,
Settings itself — fully. It does **not** yet change the gameplay: the
Letters/Numbers/Conundrum rounds and the solver's dictionary are
English-only for now, so switching languages changes what the app looks
like, not which words or numbers it works with. Closing that gap is
tracked for a future release — see the "Known limitations" note in
[CHANGELOG.md](https://github.com/iainchesworth/CountdownSolver/blob/main/CHANGELOG.md)
for the current status.
