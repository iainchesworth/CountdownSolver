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
    readonly property var keyRows: ["QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"]

    signal solveRequested()
    signal cleared()

    function addLetter(ch) { if (letters.length < 9) { letters = letters.concat([ch]) } }
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
