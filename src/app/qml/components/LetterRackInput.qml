import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Shared 9-letter rack input for LettersPage/ConundrumPage: physical
// A-Z/Backspace/Enter/Escape key handling plus the on-screen QWERTY keyboard
// visual. The rack itself is exposed via `letters`; solveRequested()/
// cleared() let the embedding page react to Enter/Escape without this
// component knowing which solver endpoint is being driven.
Item {
    id: root

    // Set to true only while the embedding page is the visible StackLayout
    // item, e.g. `active: StackLayout.isCurrentItem` evaluated at the page
    // root (that attached property only resolves for a direct StackLayout
    // child, so the page must forward it in rather than this component
    // reading it itself).
    property bool active: false
    focus: active

    property var letters: []
    // How many letters a rack holds in the active language - 9 for most,
    // 10 for French. The embedding page binds this to solver.rackSize().
    property int maxLetters: 9

    // Real regional keyboard layouts as a low-cost polish touch - French's
    // alphabet folds every accent away so AZERTY needs no extra keys, but
    // German's Ä/Ö/Ü and Spanish's Ñ are distinct alphabet slots that must
    // be reachable, so those languages get an extra key/row for them.
    // Physical keyboard input (Keys.onPressed below) stays ASCII-only for
    // every language - the on-screen keyboard is the complete/authoritative
    // input method, not just a QWERTY convenience.
    // Arabic/Hebrew/Yiddish rows are authored in logical (reading-order)
    // sequence, same as every LTR row above - the app's global
    // LayoutMirroring (Main.qml) already flips each row's RowLayout visually
    // for RTL, so no manual reversal is needed here, same mechanism as every
    // other mirrored control in the app. These aren't a physical-keyboard-
    // accurate scancode layout (no single standard exists across regions
    // the way QWERTY/AZERTY/QWERTZ do) - just the full letter set split into
    // rows for on-screen use, same "original design, no reference to copy"
    // caveat as these three languages' rack/alphabet rules generally.
    readonly property var keyRowsByLanguage: ({
        "fr": ["AZERTYUIOP", "QSDFGHJKLM", "WXCVBN"],
        "de": ["QWERTZUIOP", "ASDFGHJKL", "YXCVBNM", "ÄÖÜ"],
        "es": ["QWERTYUIOP", "ASDFGHJKLÑ", "ZXCVBNM"],
        "ar": ["ابتثجحخدذر", "زسشصضطظعغف", "قكلمنهوية"],
        "he": ["אבגדהוזח", "טיכלמנסע", "פצקרשת"],
        // No dedicated row for Yiddish's three extra digraph letters
        // (tsvey-vovn/vov-yod/tsvey-yudn - see fold_yiddish() in
        // alphabet.cpp): unlike German's Ä/Ö/Ü (a single distinct alphabet
        // slot each, unreachable any other way), every Yiddish digraph is
        // just two base letters already on this keyboard (vav+vav,
        // vav+yod, yod+yod) pressed in sequence - same as typing "sh" in
        // English takes two keystrokes. Reuses the Hebrew rows as-is.
        "yi": ["אבגדהוזח", "טיכלמנסע", "פצקרשת"],
    })
    // Guarded against a null languageManager for the same reason as
    // Theme.qml's `sans` - falls back to QWERTY until the next re-evaluation.
    readonly property var keyRows: (languageManager && keyRowsByLanguage[languageManager.currentLanguage])
                                    || ["QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"]

    signal solveRequested()
    signal cleared()

    function addLetter(ch) { if (letters.length < maxLetters) { letters = letters.concat([ch]) } }
    function backspace()   { letters = letters.slice(0, -1) }
    function clearAll()    { letters = []; root.cleared() }

    Keys.onPressed: function (event) {
        if (event.key >= Qt.Key_A && event.key <= Qt.Key_Z) {
            root.addLetter(String.fromCharCode(event.key))
            event.accepted = true
        } else if (event.key === Qt.Key_Backspace) {
            root.backspace()
            event.accepted = true
        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
            root.solveRequested()
            event.accepted = true
        } else if (event.key === Qt.Key_Escape) {
            root.clearAll()
            event.accepted = true
        }
    }

    implicitWidth: kb.implicitWidth + 36
    implicitHeight: kb.implicitHeight + 36

    Card {
        anchors.fill: parent
        ColumnLayout {
            id: kb
            anchors.fill: parent; anchors.margins: 18
            spacing: 6
            Repeater {
                model: root.keyRows
                delegate: RowLayout {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: 5
                    property string rowStr: modelData
                    Repeater {
                        model: rowStr.length
                        delegate: PadButton {
                            implicitWidth: 36; implicitHeight: 44
                            mono: false; fontSize: 15
                            text: rowStr.charAt(index)
                            onClicked: root.addLetter(rowStr.charAt(index))
                        }
                    }
                }
            }
        }
    }
}
