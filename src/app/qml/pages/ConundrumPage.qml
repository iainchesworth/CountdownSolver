import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Conundrum. Delegates solving to the C++ `solver`. See Solver.h.
Item {
    id: root
    property var letters: ["O", "C", "T", "N", "U", "D", "W", "O", "N"]
    property var result: null   // { found, answers[] }
    readonly property var keyRows: ["QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"]

    function recalc() {
        if (typeof solver === "undefined" || !solver) { result = null; return }
        result = solver.solveConundrum(root.letters.join(""))
    }
    function addLetter(ch) { if (letters.length < 9) { letters = letters.concat([ch]); recalc() } }
    function backspace()   { letters = letters.slice(0, -1); recalc() }
    function clearAll()    { letters = []; result = null }
    function randomConundrum() {
        if (typeof solver !== "undefined" && solver && solver.randomConundrum)
            letters = solver.randomConundrum().split("")
        recalc()
    }
    Component.onCompleted: recalc()

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 24; anchors.rightMargin: 24; anchors.bottomMargin: 24
        spacing: 18

        // ---- left: input ----
        ColumnLayout {
            Layout.preferredWidth: 456
            Layout.fillHeight: true
            spacing: 16

            Card {
                Layout.fillWidth: true
                implicitHeight: cc.implicitHeight + 36
                ColumnLayout {
                    id: cc
                    anchors.fill: parent; anchors.margins: 18
                    spacing: 12
                    SectionLabel { text: "Scrambled nine" }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 6
                        Repeater {
                            model: 9
                            delegate: Tile {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 58
                                mono: false
                                label: index < root.letters.length ? root.letters[index] : ""
                            }
                        }
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                implicitHeight: ck.implicitHeight + 36
                ColumnLayout {
                    id: ck
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

            RowLayout {
                Layout.fillWidth: true; spacing: 9
                FlatButton { Layout.fillWidth: true; primary: true; text: "\u21bb Random conundrum"; onClicked: root.randomConundrum() }
                FlatButton { text: "\u232b"; onClicked: root.backspace() }
                FlatButton { text: "Clear"; onClicked: root.clearAll() }
            }
            Item { Layout.fillHeight: true }
        }

        // ---- right: solution ----
        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 16
                visible: root.result && root.result.found
                SectionLabel { Layout.alignment: Qt.AlignHCenter; text: "Solution" }
                Grid {
                    Layout.alignment: Qt.AlignHCenter
                    columns: 3; spacing: 7
                    Repeater {
                        model: (root.result && root.result.found) ? root.result.answers[0].toUpperCase().split("") : []
                        delegate: Rectangle {
                            width: 44; height: 52; radius: 9; color: Theme.accent
                            Text { anchors.centerIn: parent; text: modelData; color: Theme.accentInk; font.family: Theme.sans; font.pixelSize: 24; font.weight: Font.DemiBold }
                        }
                    }
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    visible: root.result && root.result.answers && root.result.answers.length > 1
                    text: "Also valid: " + (root.result && root.result.answers ? root.result.answers.slice(1).join(", ").toUpperCase() : "")
                    color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13
                }
            }

            // empty / not-found state
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 12
                visible: !root.result || !root.result.found
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 52; height: 52; radius: 12; color: "transparent"
                    border.width: 2; border.color: Theme.tileBorder
                    Text { anchors.centerIn: parent; text: "?"; color: Theme.faint; font.family: Theme.sans; font.pixelSize: 22; font.weight: Font.DemiBold }
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 260
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: (root.letters.length < 9)
                          ? "Enter all nine letters to solve the conundrum."
                          : "No single word in the list uses these letters."
                    color: Theme.muted; font.family: Theme.sans; font.pixelSize: 15
                }
            }
        }
    }
}
