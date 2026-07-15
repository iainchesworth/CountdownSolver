import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Letters game. Delegates solving to the C++ `solver`. See Solver.h.
// Solving only happens when the user presses Solve (or Enter) - typing
// letters never solves implicitly.
Item {
    id: root
    focus: StackLayout.isCurrentItem
    property var letters: []
    property var result: null   // { total, shown, maxLen, longest[], groups[{len,words[]}] }

    property string view: "input"
    readonly property bool hasResult: result !== null
    // See NumbersPage.qml's identical helper - only invalidates on the three
    // new form factors, so desktop/tablet-landscape's existing "stale result
    // stays until Solve/Clear" behaviour is untouched.
    function invalidateIfStale() {
        if (result !== null && Metrics.compactIndex >= 0) result = null
    }

    function recalc() {
        if (typeof solver === "undefined" || !solver) { result = null; return }
        result = solver.solveLetters(letters.join(""), AppState.minLen, AppState.maxResults)
    }
    function addLetter(ch) {
        if (letters.length < (solver.dictionariesReady, solver.rackSize())) {
            letters = letters.concat([ch])
            invalidateIfStale()
        }
    }
    function backspace() { letters = letters.slice(0, -1); invalidateIfStale() }
    function clearAll()  { letters = []; result = null }
    function randomRack() {
        if (typeof solver !== "undefined" && solver && solver.randomRack)
            letters = solver.randomRack().split("")
        recalc()
        root.view = "results"
    }
    Connections {
        target: AppState
        function onMinLenChanged()            { if (root.result !== null) root.recalc() }
        function onMaxResultsChanged()        { if (root.result !== null) root.recalc() }
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

    Keys.onPressed: function (event) {
        if (event.key >= Qt.Key_A && event.key <= Qt.Key_Z) {
            root.addLetter(String.fromCharCode(event.key))
            event.accepted = true
        } else if (event.key === Qt.Key_Backspace) {
            root.backspace()
            event.accepted = true
        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
            root.recalc()
            root.view = "results"
            event.accepted = true
        } else if (event.key === Qt.Key_Escape) {
            root.clearAll()
            event.accepted = true
        }
    }

    // ---- shared building blocks (used across all form-factor layouts) ----

    component RackTileCard: Card {
        Layout.fillWidth: true
        cornerRadius: Metrics.compactIndex < 0 ? Theme.radiusCard : Metrics.cardRadius.value[Metrics.compactIndex]
        implicitHeight: lc.implicitHeight + (Metrics.compactIndex < 0 ? 18 : Metrics.cardPadding.value[Metrics.compactIndex]) * 2
        ColumnLayout {
            id: lc
            anchors.fill: parent
            anchors.margins: Metrics.compactIndex < 0 ? 18 : Metrics.cardPadding.value[Metrics.compactIndex]
            spacing: 12
            SectionLabel {
                text: qsTr("%1 letters").arg((solver.dictionariesReady, solver.rackSize()))
            }
            RowLayout {
                Layout.fillWidth: true; spacing: 6
                Repeater {
                    model: (solver.dictionariesReady, solver.rackSize())
                    delegate: Tile {
                        Layout.fillWidth: true
                        Layout.preferredHeight: Metrics.compactIndex < 0 ? 58 : Metrics.rackTile.size[Metrics.compactIndex]
                        fontSize: Metrics.compactIndex < 0 ? 24 : Metrics.rackTile.font[Metrics.compactIndex]
                        mono: false
                        label: index < root.letters.length ? root.letters[index] : ""
                    }
                }
            }
        }
    }

    component ActionButtons: ColumnLayout {
        spacing: 9
        RowLayout {
            Layout.fillWidth: true; spacing: 9
            FlatButton {
                Layout.fillWidth: true
                implicitHeight: Metrics.touchSize(Metrics.pick(Metrics.actionButton.size, 42))
                text: "↻ " + qsTr("Random rack"); onClicked: root.randomRack()
            }
            FlatButton {
                implicitHeight: Metrics.touchSize(Metrics.pick(Metrics.actionButton.size, 42))
                text: "←"; onClicked: root.backspace()
            }
            FlatButton {
                implicitHeight: Metrics.touchSize(Metrics.pick(Metrics.actionButton.size, 42))
                text: qsTr("Clear"); onClicked: root.clearAll()
            }
        }
        FlatButton {
            Layout.fillWidth: true
            implicitHeight: Metrics.touchSize(Metrics.pick(Metrics.solveButton.size, 42))
            primary: true
            text: qsTr("Solve")
            enabled: root.letters.length > 0
            onClicked: { root.recalc(); root.view = "results" }
        }
    }

    component LongestWordsLabel: Text {
        text: (root.result && root.result.longest && root.result.longest.length > 1
               ? qsTr("Longest words")
               : qsTr("Longest word"))
              + " · " + qsTr("%n letter(s)", "", root.result ? root.result.maxLen : 0)
        color: Theme.faint; font.family: Theme.mono; font.pixelSize: 11; font.letterSpacing: 1.2
        font.capitalization: Font.AllUppercase
    }

    // Stacked: label above a wrapping row of chips (desktop/tablet-landscape/
    // tablet-portrait/phone-portrait).
    component LongestWordsStacked: ColumnLayout {
        spacing: 0
        LongestWordsLabel {}
        Flow {
            Layout.fillWidth: true; Layout.topMargin: 12; spacing: 8
            Repeater {
                model: root.result ? root.result.longest : []
                delegate: AccentChip {}
            }
        }
    }

    // Packed into one row (phone-landscape only) - label, wrapping chips,
    // and the letter-count all on one line to save vertical space.
    component LongestWordsRow: RowLayout {
        spacing: 14
        LongestWordsLabel {}
        Flow {
            Layout.fillWidth: true; spacing: 8
            Repeater {
                model: root.result ? root.result.longest : []
                delegate: AccentChip {}
            }
        }
    }

    component AllWordsHeader: RowLayout {
        Layout.fillWidth: true
        SectionLabel { Layout.fillWidth: true; text: qsTr("All valid words") }
        Text {
            text: root.result ? (root.result.shown < root.result.total
                  ? qsTr("showing %1 of %2").arg(root.result.shown).arg(root.result.total)
                  : qsTr("%n found", "", root.result.total)) : ""
            color: Theme.faint; font.family: Theme.mono; font.pixelSize: 12
        }
    }

    component WordGroups: ColumnLayout {
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

    component ResultsEmptyState: ColumnLayout {
        spacing: 10
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

    // ---- per-form-factor layout ----

    Loader {
        anchors.fill: parent
        sourceComponent: Metrics.formFactor === FormFactor.phoneLandscape ? phoneLandscapeLayout
                        : (Metrics.formFactor === FormFactor.tabletPortrait
                           || Metrics.formFactor === FormFactor.phonePortrait) ? portraitLayout
                        : desktopLayout
    }

    // Desktop/tablet-landscape - byte-for-byte the original side-by-side layout.
    Component {
        id: desktopLayout
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 24; anchors.rightMargin: 24; anchors.bottomMargin: 24
            spacing: 18

            ColumnLayout {
                Layout.preferredWidth: 456
                Layout.fillWidth: false
                Layout.fillHeight: true
                spacing: 16

                RackTileCard {}
                OnScreenKeyboard {
                    Layout.fillWidth: true
                    onKeyPressed: ch => root.addLetter(ch)
                }
                ActionButtons { Layout.fillWidth: true }
                Item { Layout.fillHeight: true }
            }

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
                        LongestWordsStacked {}
                        Rectangle { Layout.fillWidth: true; Layout.topMargin: 16; Layout.bottomMargin: 16; height: 1; color: Theme.border }
                        AllWordsHeader {}
                        WordGroups {}
                    }
                }
                ResultsEmptyState { anchors.centerIn: parent; visible: !root.result || root.result.total === 0 }
            }
        }
    }

    // Tablet-portrait/phone-portrait - single column, Input/Results toggled
    // by `view` (the toggle bar itself is drawn by Main.qml, above this page).
    Component {
        id: portraitLayout
        Item {
            Flickable {
                anchors.fill: parent
                anchors.margins: Metrics.headerPadding.side[Metrics.compactIndex]
                visible: root.view === "input"
                clip: true
                contentHeight: inputCol.implicitHeight
                boundsBehavior: Flickable.StopAtBounds
                ColumnLayout {
                    id: inputCol
                    width: parent.width
                    spacing: 16
                    RackTileCard {}
                    OnScreenKeyboard {
                        Layout.fillWidth: true
                        onKeyPressed: ch => root.addLetter(ch)
                    }
                    ActionButtons { Layout.fillWidth: true }
                    Item { Layout.preferredHeight: 4 }
                }
            }

            Card {
                anchors.fill: parent
                anchors.margins: Metrics.headerPadding.side[Metrics.compactIndex]
                cornerRadius: Metrics.cardRadius.value[Metrics.compactIndex]
                visible: root.view === "results"
                clip: true

                Flickable {
                    anchors.fill: parent
                    anchors.margins: Metrics.cardPadding.value[Metrics.compactIndex]
                    contentHeight: resCol.implicitHeight
                    clip: true
                    visible: root.result && root.result.total > 0
                    boundsBehavior: Flickable.StopAtBounds
                    ColumnLayout {
                        id: resCol
                        width: parent.width
                        spacing: 16
                        LongestWordsStacked {}
                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
                        AllWordsHeader {}
                        WordGroups {}
                    }
                }
                ResultsEmptyState { anchors.centerIn: parent; visible: !root.result || root.result.total === 0 }
            }
        }
    }

    // Phone-landscape - fixed-width tile/keyboard pane split, results card
    // with the "longest" block packed into one row.
    Component {
        id: phoneLandscapeLayout
        Item {
            RowLayout {
                anchors.fill: parent
                anchors.margins: Metrics.headerPadding.side[Metrics.compactIndex]
                spacing: 12
                visible: root.view === "input"

                // Wrapped in a Flickable, not just a fixed-width ColumnLayout -
                // ~390px of window height in landscape doesn't always fit the
                // tile card + action row, so it scrolls instead of clipping
                // under the bottom tab bar.
                Flickable {
                    Layout.preferredWidth: 320
                    Layout.fillWidth: false
                    Layout.fillHeight: true
                    clip: true
                    contentHeight: leftCol.implicitHeight
                    boundsBehavior: Flickable.StopAtBounds
                    ColumnLayout {
                        id: leftCol
                        width: parent.width
                        spacing: 12
                        RackTileCard {}
                        ActionButtons { Layout.fillWidth: true }
                    }
                }
                OnScreenKeyboard {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.alignment: Qt.AlignVCenter
                    onKeyPressed: ch => root.addLetter(ch)
                }
            }

            Card {
                anchors.fill: parent
                anchors.margins: Metrics.headerPadding.side[Metrics.compactIndex]
                cornerRadius: Metrics.cardRadius.value[Metrics.compactIndex]
                visible: root.view === "results"
                clip: true

                Flickable {
                    anchors.fill: parent
                    anchors.margins: Metrics.cardPadding.value[Metrics.compactIndex]
                    contentHeight: resCol.implicitHeight
                    clip: true
                    visible: root.result && root.result.total > 0
                    boundsBehavior: Flickable.StopAtBounds
                    ColumnLayout {
                        id: resCol
                        width: parent.width
                        spacing: 14
                        LongestWordsRow {}
                        Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
                        AllWordsHeader {}
                        WordGroups {}
                    }
                }
                ResultsEmptyState { anchors.centerIn: parent; visible: !root.result || root.result.total === 0 }
            }
        }
    }
}
