# Countdown Solver — UI Design Spec

Clean, calm, native-desktop utility. Two type families, one teal accent, warm
neutral surfaces. Light is the default; a full dark theme ships alongside it.
All values below live in `qml/Theme.qml` (colour/type/geometry) — treat that
singleton as the source of truth and reference tokens rather than hard-coding.

---

## 1. Colour tokens

| Token         | Light     | Dark      | Use                                  |
|---------------|-----------|-----------|--------------------------------------|
| `desk`        | `#e7e6e1` | `#131211` | Backdrop behind the window           |
| `bg`          | `#f7f6f3` | `#232220` | Content area / inset wells           |
| `panel`       | `#ffffff` | `#2b2a27` | Cards                                |
| `sidebar`     | `#efeee9` | `#1b1a18` | Left navigation rail                 |
| `border`      | `#e2e0d8` | `#3b3934` | Hairlines, card borders              |
| `ink`         | `#2c2b28` | `#eceae4` | Primary text                         |
| `muted`       | `#6f6d64` | `#a6a399` | Secondary text                       |
| `faint`       | `#9a978d` | `#767369` | Labels, meta, placeholders           |
| `accent`      | `#2f7d7b` | `#54b8b0` | Teal — target, active nav, solutions |
| `accentInk`   | `#ffffff` | `#0e1f1e` | Text/glyphs on accent                |
| `accentSoft`  | `#e4efee` | `#24403e` | Large-number pads, step badges       |
| `tile`        | `#ffffff` | `#34322e` | Filled tiles, pad buttons            |
| `tileInk`     | `#2c2b28` | `#f2f0ea` | Text on tiles                        |
| `tileBorder`  | `#d9d6cd` | `#46443d` | Tile / pad borders                   |
| `warnBg`      | `#f0e6d6` | `#3a3324` | "N away" badge background            |
| `warnInk`     | `#9a6a2c` | `#d8b06a` | "N away" badge text                  |

Accent stays a single hue; light/dark differ only in lightness. No gradients.

---

## 2. Typography

Families: **IBM Plex Sans** (UI) and **IBM Plex Mono** (numbers, tiles, section
labels, working steps). Fallback to system UI / monospace if not bundled.

| Role               | Family | Size | Weight    | Notes                              |
|--------------------|--------|------|-----------|------------------------------------|
| Page title         | Sans   | 22   | Bold 700  |                                    |
| Page subtitle      | Sans   | 14   | Reg 400   | `muted`                            |
| Section label      | Mono   | 11   | SemiB 600 | UPPERCASE, letter-spacing 1.2      |
| Nav item           | Sans   | 14   | 500 / 600 | 600 when active                    |
| Nav badge glyph    | Mono   | 12   | SemiB 600 |                                    |
| Number tile        | Mono   | 24   | SemiB 600 |                                    |
| Letter/conundrum tile | Sans | 24  | SemiB 600 | UPPERCASE                          |
| Target display     | Mono   | 28   | SemiB 600 | letter-spacing 3                   |
| Result value       | Mono   | 46   | SemiB 600 |                                    |
| Working step       | Mono   | 18   | Med 500   |                                    |
| Word chip          | Sans   | 14   | Med 500   | UPPERCASE, letter-spacing 0.6      |
| Longest-word chip  | Sans   | 20   | SemiB 600 | UPPERCASE, letter-spacing 1.5      |
| Setting title/body | Sans   | 15 / 13 | 600 / 400 |                                 |

---

## 3. Geometry & layout

- **Window**: 1140 × 740 default, min 900 × 620. Background `bg`. Native OS frame.
- **Sidebar**: fixed width **214**, background `sidebar`, 12 padding.
- **Content**: 24 padding on left/right/bottom; header block at top.
- **Two-column pages** (Numbers/Letters/Conundrum): left input column fixed
  **456** wide, right results card fills remaining width; **18** gap.
- **Radii**: window frame 14 (OS), cards `radiusCard` 12, controls/tiles
  `radiusControl` 9–10, chips 7, badges/pills 20.
- **Card padding**: 18 (input cards) / 22 (results) / 20 (settings).
- **Gaps**: tiles 6–8, pad grids 6–8, chip flows 7–8, control stacks 16.

### Component sizes
| Element              | Size                          |
|----------------------|-------------------------------|
| Number tile          | fill × 62                     |
| Letter/conundrum tile| fill × 58                     |
| Large number pad     | fill × 46                     |
| Small number pad     | fill × 42                     |
| Target digit key     | fill × 38                     |
| Keyboard key         | 36 × 44                       |
| Nav item             | fill × 44, 26×26 badge, 3px left bar |
| Secondary button     | × 42, 16 h-padding            |
| Working step row     | fill × 44, 22×22 index badge  |
| Word chip            | auto × 30                     |
| Longest chip         | auto × 44                     |
| Conundrum answer tile| 44 × 52, 3-column grid        |

### Iconography
No icon library. Nav uses mono glyph badges: Numbers `12`, Letters `Aa`,
Conundrum `?`, Settings `≡`. Buttons use `↻` (random), `⌫` (backspace).

### States
- **Nav active**: `accentSoft` fill, 3px `accent` left bar, `accent` badge/label.
- **Pad hover/down**: slight `Qt.darker` shift on the fill.
- **Tile filled vs empty**: filled = `tile` + 1px border; empty = `bg` + 2px
  border (dashed in the web ref — solid in QML unless drawn with Qt Quick Shapes).
- **Numbers badge**: exact = `accent`/`accentInk`; otherwise "N away" in
  `warnBg`/`warnInk` when *Flag inexact* is on, else quiet `bg`/`muted`.

---

## 4. Screens

1. **Numbers** — six selection tiles; pads for 25/50/75/100 (accent) + 1–10;
   accent target display with a 0–9 keypad. Result card: big value, exact/away
   badge, numbered working steps. Empty state prompts for what's missing.
2. **Letters** — nine tiles; full QWERTY keyboard. Result card: longest word(s)
   as accent chips, then all words grouped by length (desc) as outline chips,
   with a "showing X of Y" count. Scrolls.
3. **Conundrum** — nine scrambled tiles; keyboard. Solution revealed as a
   3-column grid of accent tiles; extra valid answers listed below.
4. **Settings** — Appearance (theme), Solver (min word length, flag-inexact
   toggle, max-words slider), Dictionary (sample vs full list).

---

## 5. Solver contract (QML ⇄ C++)

The QML calls a `solver` context property. Exact return shapes are in
`src/Solver.h`. Summary:

- `solveNumbers(numbers:[int], target:int)` → `{ value, diff, exact, steps:[string] }`
- `solveLetters(rack:string, minLen:int, maxResults:int)` →
  `{ total, shown, maxLen, longest:[string], groups:[{ len, words:[string] }] }`
- `solveConundrum(letters:string)` → `{ found, answers:[string] }`
- `randomRack()` / `randomConundrum()` → `string` (nine letters; `""` disables)

Working-step string format: `"<hi> <op> <lo> = <result>"` with `op` ∈
`+ − × ÷` (U+2212, U+00D7, U+00F7). The numbers search should carry a node
budget so it never blocks the UI thread (see the reference note in `Solver.cpp`).
