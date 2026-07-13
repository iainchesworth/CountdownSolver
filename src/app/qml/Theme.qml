pragma Singleton
import QtQuick

// Design tokens for the Countdown Solver. Single source of truth for colour,
// type and geometry. Flip `dark` to switch the whole app between themes.
QtObject {
    property bool dark: false

    // ---- palette (light / dark) ----
    readonly property color desk:       dark ? "#131211" : "#e7e6e1"   // window backdrop
    readonly property color bg:         dark ? "#232220" : "#f7f6f3"   // content area
    readonly property color panel:      dark ? "#2b2a27" : "#ffffff"   // cards
    readonly property color sidebar:    dark ? "#1b1a18" : "#efeee9"
    readonly property color border:     dark ? "#3b3934" : "#e2e0d8"
    readonly property color ink:        dark ? "#eceae4" : "#2c2b28"   // primary text
    readonly property color muted:      dark ? "#a6a399" : "#6f6d64"   // secondary text
    readonly property color faint:      dark ? "#767369" : "#9a978d"   // labels / meta
    readonly property color accent:     dark ? "#54b8b0" : "#2f7d7b"   // teal
    readonly property color accentInk:  dark ? "#0e1f1e" : "#ffffff"   // text on accent
    readonly property color accentSoft: dark ? "#24403e" : "#e4efee"
    readonly property color tile:       dark ? "#34322e" : "#ffffff"
    readonly property color tileInk:    dark ? "#f2f0ea" : "#2c2b28"
    readonly property color tileBorder: dark ? "#46443d" : "#d9d6cd"
    readonly property color warnBg:     dark ? "#3a3324" : "#f0e6d6"   // "N away" badge
    readonly property color warnInk:    dark ? "#d8b06a" : "#9a6a2c"

    // ---- type ----
    // Arabic/Hebrew/Yiddish need a script-appropriate typeface - IBM Plex
    // Sans has no Arabic or Hebrew glyph coverage. Noto Sans Arabic/Hebrew
    // are bundled as application fonts (see main.cpp) for exactly this.
    // Bound to languageManager.currentLanguage (a NOTIFYing property) rather
    // than Qt.locale(), so it re-evaluates on every language switch.
    readonly property var rtlFonts: ({
        "ar": "Noto Sans Arabic",
        "he": "Noto Sans Hebrew",
        "yi": "Noto Sans Hebrew",
    })
    readonly property string sans: rtlFonts[languageManager.currentLanguage] || "IBM Plex Sans"
    readonly property string mono: "IBM Plex Mono"

    // ---- geometry ----
    readonly property int radiusCard:    12
    readonly property int radiusControl: 9
    readonly property int gap:  8
    readonly property int pad: 18
}
