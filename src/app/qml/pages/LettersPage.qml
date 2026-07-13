import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Letters game. Delegates solving to the C++ `solver`. See Solver.h.
Item {
    id: root
    property var letters: ["R", "E", "T", "A", "I", "N", "D", "O", "S"]
    property var result: null   // { total, shown, maxLen, longest[], groups[{len,words[]}] }
    readonly property var keyRows: ["QWERTYUIOP", "ASDFGHJKL", "ZXCVBNM"]

    function recalc() {
        if (typeof solver === "undefined" || !solver) { result = null; return }
        result = solver.solveLetters(root.letters.join(""), AppState.minLen, AppState.maxResults)
    }
    function addLetter(ch) { if (letters.length < 9) { letters = letters.concat([ch]); recalc() } }
    function backspace()   { letters = letters.slice(0, -1); recalc() }
    function clearAll()    { letters = []; result = null }
    function randomRack() {
        if (typeof solver !== "undefined" && solver && solver.randomRack)
            letters = solver.randomRack().split("")
        recalc()
    }
    Component.onCompleted: recalc()
    Connections {
        target: AppState
        function onMinLenChanged()          { root.recalc() }
        function onMaxResultsChanged()      { root.recalc() }
        function onUseFullDictionaryChanged() { root.recalc() }
    }

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
                implicitHeight: lc.implicitHeight + 36
                ColumnLayout {
                    id: lc
                    anchors.fill: parent; anchors.margins: 18
                    spacing: 12
                    SectionLabel { text: "Nine letters" }
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
                implicitHeight: kb.implicitHeight + 36
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

            RowLayout {
                Layout.fillWidth: true; spacing: 9
                FlatButton { Layout.fillWidth: true; primary: true; text: "\u21bb Random rack"; onClicked: root.randomRack() }
                FlatButton { text: "\u232b"; onClicked: root.backspace() }
                FlatButton { text: "Clear"; onClicked: root.clearAll() }
            }
            Item { Layout.fillHeight: true }
        }

        // ---- right: results ----
        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true

            Flickable {
                anchors.fill: parent; anchors.margins: 22
                contentHeight: resCol.implicitHeight
                clip: true
                visible: root.result && root.result.total > 0
                boundsBehavior: Flickable.StopAtBounds

                ColumnLayout {
                    id: resCol
                    width: parent.width
                    spacing: 0

                    SectionLabel {
                        text: (root.result && root.result.longest && root.result.longest.length > 1 ? "Longest words \u00b7 " : "Longest word \u00b7 ")
                              + (root.result ? root.result.maxLen : 0) + " letters"
                    }
                    Flow {
                        Layout.fillWidth: true; Layout.topMargin: 12; spacing: 8
                        Repeater {
                            model: root.result ? root.result.longest : []
                            delegate: Rectangle {
                                radius: 9; color: Theme.accent; height: 44
                                width: lw.implicitWidth + 36
                                Text {
                                    id: lw; anchors.centerIn: parent; text: modelData
                                    color: Theme.accentInk; font.family: Theme.sans; font.pixelSize: 20
                                    font.weight: Font.DemiBold; font.capitalization: Font.AllUppercase; font.letterSpacing: 1.5
                                }
                            }
                        }
                    }

                    Rectangle { Layout.fillWidth: true; Layout.topMargin: 16; Layout.bottomMargin: 16; height: 1; color: Theme.border }

                    RowLayout {
                        Layout.fillWidth: true
                        SectionLabel { Layout.fillWidth: true; text: "All valid words" }
                        Text {
                            text: root.result ? (root.result.shown < root.result.total
                                  ? "showing " + root.result.shown + " of " + root.result.total
                                  : root.result.total + " found") : ""
                            color: Theme.faint; font.family: Theme.mono; font.pixelSize: 12
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; Layout.topMargin: 12; spacing: 14
                        Repeater {
                            model: root.result ? root.result.groups : []
                            delegate: ColumnLayout {
                                Layout.fillWidth: true; spacing: 8
                                property var grp: modelData
                                Text {
                                    text: grp.len + " letters (" + grp.words.length + ")"
                                    color: Theme.muted; font.family: Theme.mono; font.pixelSize: 12; font.weight: Font.DemiBold
                                }
                                Flow {
                                    Layout.fillWidth: true; spacing: 7
                                    Repeater {
                                        model: grp.words
                                        delegate: Rectangle {
                                            radius: 7; height: 30; width: cw.implicitWidth + 22
                                            color: Theme.bg; border.width: 1; border.color: Theme.border
                                            Text {
                                                id: cw; anchors.centerIn: parent; text: modelData
                                                color: Theme.ink; font.family: Theme.sans; font.pixelSize: 14
                                                font.capitalization: Font.AllUppercase; font.letterSpacing: 0.6
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // empty state
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 10
                visible: !root.result || root.result.total === 0
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 52; height: 52; radius: 12; color: "transparent"
                    border.width: 2; border.color: Theme.tileBorder
                    Text { anchors.centerIn: parent; text: "Aa"; color: Theme.faint; font.family: Theme.sans; font.pixelSize: 20; font.weight: Font.DemiBold }
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 240
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: (root.result && root.result.total === 0)
                          ? "No words " + AppState.minLen + "+ letters long. Try a random rack."
                          : "Type or tap nine letters to find words."
                    color: Theme.muted; font.family: Theme.sans; font.pixelSize: 15
                }
            }
        }
    }
}
