import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Letters game. Delegates solving to the C++ `solver`. See Solver.h.
// Solving only happens when the user presses Solve (or Enter) - typing
// letters never solves implicitly.
Item {
    id: root
    readonly property bool isCurrentPage: StackLayout.isCurrentItem
    property alias letters: rackInput.letters
    property var result: null   // { total, shown, maxLen, longest[], groups[{len,words[]}] }

    function recalc() {
        if (typeof solver === "undefined" || !solver) { result = null; return }
        result = solver.solveLetters(rackInput.letters.join(""), AppState.minLen, AppState.maxResults)
    }
    function addLetter(ch) { rackInput.addLetter(ch) }
    function backspace()   { rackInput.backspace() }
    function clearAll()    { rackInput.clearAll() }
    function randomRack() {
        if (typeof solver !== "undefined" && solver && solver.randomRack)
            rackInput.letters = solver.randomRack().split("")
        recalc()
    }
    Connections {
        target: AppState
        function onMinLenChanged()          { if (root.result !== null) root.recalc() }
        function onMaxResultsChanged()      { if (root.result !== null) root.recalc() }
        function onUseFullDictionaryChanged() { if (root.result !== null) root.recalc() }
    }
    // The rack's own length constraint (solver.rackSize()) can change on a
    // language switch (French draws 10, not 9) - a rack built for the old
    // language may no longer be a valid length, so clear rather than leave
    // stale state.
    Connections {
        target: languageManager
        function onCurrentLanguageChanged() { root.clearAll() }
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
                implicitHeight: lc.implicitHeight + 36
                ColumnLayout {
                    id: lc
                    anchors.fill: parent; anchors.margins: 18
                    spacing: 12
                    SectionLabel {
                        text: qsTr("%1 letters").arg(
                            (solver.dictionariesReady, solver.rackSize()))
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 6
                        Repeater {
                            model: (solver.dictionariesReady, solver.rackSize())
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
                maxLetters: (solver.dictionariesReady, solver.rackSize())
                onSolveRequested: root.recalc()
                onCleared: root.result = null
            }

            RowLayout {
                Layout.fillWidth: true; spacing: 9
                FlatButton { Layout.fillWidth: true; text: "\u21bb " + qsTr("Random rack"); onClicked: root.randomRack() }
                FlatButton { text: "\u232b"; onClicked: rackInput.backspace() }
                FlatButton { text: qsTr("Clear"); onClicked: rackInput.clearAll() }
            }
            FlatButton {
                Layout.fillWidth: true
                primary: true
                text: qsTr("Solve")
                enabled: rackInput.letters.length > 0
                onClicked: root.recalc()
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
                        text: (root.result && root.result.longest && root.result.longest.length > 1
                               ? qsTr("Longest words")
                               : qsTr("Longest word"))
                              + " \u00b7 " + qsTr("%n letter(s)", "", root.result ? root.result.maxLen : 0)
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
                        SectionLabel { Layout.fillWidth: true; text: qsTr("All valid words") }
                        Text {
                            text: root.result ? (root.result.shown < root.result.total
                                  ? qsTr("showing %1 of %2").arg(root.result.shown).arg(root.result.total)
                                  : qsTr("%n found", "", root.result.total)) : ""
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
                                    text: qsTr("%n letter(s) (%1)", "", grp.len).arg(grp.count)
                                          + (grp.count > grp.words.length ? " · " + qsTr("showing %1").arg(grp.words.length) : "")
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
                          ? qsTr("No words %1+ letters long. Try a random rack.").arg(AppState.minLen)
                          : (root.letters.length > 0
                             ? qsTr("Press Solve to find words.")
                             : qsTr("Type or tap %1 letters, then press Solve.").arg(
                                 (solver.dictionariesReady, solver.rackSize())))
                    color: Theme.muted; font.family: Theme.sans; font.pixelSize: 15
                }
            }
        }
    }
}
