import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Conundrum. Delegates solving to the C++ `solver`. See Solver.h.
// Solving only happens when the user presses Solve (or Enter) - typing
// letters never solves implicitly.
Item {
    id: root
    readonly property bool isCurrentPage: StackLayout.isCurrentItem
    property alias letters: rackInput.letters
    property var result: null   // { found, answers[] }

    function recalc() {
        if (typeof solver === "undefined" || !solver) { result = null; return }
        result = solver.solveConundrum(rackInput.letters.join(""))
    }
    function addLetter(ch) { rackInput.addLetter(ch) }
    function backspace()   { rackInput.backspace() }
    function clearAll()    { rackInput.clearAll() }
    function randomConundrum() {
        if (typeof solver !== "undefined" && solver && solver.randomConundrum)
            rackInput.letters = solver.randomConundrum().split("")
        recalc()
    }
    Connections {
        target: AppState
        function onUseFullDictionaryChanged() { if (root.result !== null) root.recalc() }
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 24; anchors.rightMargin: 24; anchors.bottomMargin: 24
        spacing: 18

        // ---- left: input ----
        ColumnLayout {
            Layout.preferredWidth: 456
            Layout.fillWidth: false
            Layout.fillHeight: true
            spacing: 16

            Card {
                Layout.fillWidth: true
                implicitHeight: cc.implicitHeight + 36
                ColumnLayout {
                    id: cc
                    anchors.fill: parent; anchors.margins: 18
                    spacing: 12
                    SectionLabel { text: qsTr("Scrambled nine") }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 6
                        Repeater {
                            model: 9
                            delegate: Tile {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 58
                                mono: false
                                label: index < rackInput.letters.length ? rackInput.letters[index] : ""
                            }
                        }
                    }
                }
            }

            LetterRackInput {
                id: rackInput
                Layout.fillWidth: true
                active: root.isCurrentPage
                onSolveRequested: root.recalc()
                onCleared: root.result = null
            }

            RowLayout {
                Layout.fillWidth: true; spacing: 9
                FlatButton { Layout.fillWidth: true; text: "\u21bb " + qsTr("Random conundrum"); onClicked: root.randomConundrum() }
                FlatButton { text: "\u232b"; onClicked: rackInput.backspace() }
                FlatButton { text: qsTr("Clear"); onClicked: rackInput.clearAll() }
            }
            FlatButton {
                Layout.fillWidth: true
                primary: true
                text: qsTr("Solve")
                enabled: rackInput.letters.length === 9
                onClicked: root.recalc()
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
                SectionLabel { Layout.alignment: Qt.AlignHCenter; text: qsTr("Solution") }
                Flow {
                    Layout.alignment: Qt.AlignHCenter
                    width: 300; spacing: 7
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
                    text: qsTr("Also valid: %1").arg(root.result && root.result.answers ? root.result.answers.slice(1).join(", ").toUpperCase() : "")
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
                          ? qsTr("Enter all nine letters, then press Solve.")
                          : (root.result === null
                             ? qsTr("Press Solve to reveal the solution.")
                             : qsTr("No single word in the list uses these letters."))
                    color: Theme.muted; font.family: Theme.sans; font.pixelSize: 15
                }
            }
        }
    }
}
