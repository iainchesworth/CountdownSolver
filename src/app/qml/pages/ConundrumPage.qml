import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Conundrum. Delegates solving to the C++ `solver`. See Solver.h.
// Solving only happens when the user presses Solve (or Enter) - typing
// letters never solves implicitly.
Item {
    id: root
    focus: StackLayout.isCurrentItem
    property var letters: []
    property var result: null   // { found, answers[] }

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
        result = solver.solveConundrum(letters.join(""))
    }
    function addLetter(ch) {
        if (letters.length < (solver.dictionariesReady, solver.rackSize())) {
            letters = letters.concat([ch])
            invalidateIfStale()
        }
    }
    function backspace() { letters = letters.slice(0, -1); invalidateIfStale() }
    function clearAll()  { letters = []; result = null }
    function randomConundrum() {
        if (typeof solver !== "undefined" && solver && solver.randomConundrum)
            letters = solver.randomConundrum().split("")
        recalc()
        root.view = "results"
    }
    Connections {
        target: AppState
        function onUseFullDictionaryChanged() { if (root.result !== null) root.recalc() }
    }
    // See LettersPage.qml's identical Connections - the rack's length
    // constraint can change on a language switch (French draws 10, not 9).
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
        implicitHeight: cc.implicitHeight + (Metrics.compactIndex < 0 ? 18 : Metrics.cardPadding.value[Metrics.compactIndex]) * 2
        ColumnLayout {
            id: cc
            anchors.fill: parent
            anchors.margins: Metrics.compactIndex < 0 ? 18 : Metrics.cardPadding.value[Metrics.compactIndex]
            spacing: 12
            SectionLabel {
                text: qsTr("Scrambled %1").arg((solver.dictionariesReady, solver.rackSize()))
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
                implicitHeight: Metrics.touchSize(Metrics.actionButton.size[Metrics.compactIndex])
                text: "↻ " + qsTr("Random conundrum"); onClicked: root.randomConundrum()
            }
            FlatButton {
                implicitHeight: Metrics.touchSize(Metrics.actionButton.size[Metrics.compactIndex])
                text: "←"; onClicked: root.backspace()
            }
            FlatButton {
                implicitHeight: Metrics.touchSize(Metrics.actionButton.size[Metrics.compactIndex])
                text: qsTr("Clear"); onClicked: root.clearAll()
            }
        }
        FlatButton {
            Layout.fillWidth: true
            implicitHeight: Metrics.touchSize(Metrics.solveButton.size[Metrics.compactIndex])
            primary: true
            text: qsTr("Solve")
            enabled: root.letters.length === (solver.dictionariesReady, solver.rackSize())
            onClicked: { root.recalc(); root.view = "results" }
        }
    }

    // Solution + "also valid" content, centered - the same shape at every
    // form factor, just resized via Metrics.
    component SolutionContent: ColumnLayout {
        spacing: Metrics.compactIndex < 0 ? 16 : 12
        visible: root.result && root.result.found
        SectionLabel { Layout.alignment: Qt.AlignHCenter; text: qsTr("Solution") }
        Flow {
            Layout.alignment: Qt.AlignHCenter
            width: 300; spacing: 7
            Repeater {
                model: (root.result && root.result.found) ? root.result.answers[0].toUpperCase().split("") : []
                delegate: Rectangle {
                    width: Metrics.compactIndex < 0 ? 44 : Metrics.conundrumTile.w[Metrics.compactIndex]
                    height: Metrics.compactIndex < 0 ? 52 : Metrics.conundrumTile.h[Metrics.compactIndex]
                    radius: 9; color: Theme.accent
                    Text {
                        anchors.centerIn: parent; text: modelData
                        color: Theme.accentInk; font.family: Theme.sans
                        font.pixelSize: Metrics.compactIndex < 0 ? 24 : Metrics.conundrumTile.font[Metrics.compactIndex]
                        font.weight: Font.DemiBold
                    }
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

    component ResultsEmptyState: ColumnLayout {
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
            text: (root.letters.length < (solver.dictionariesReady, solver.rackSize()))
                  ? qsTr("Enter all %1 letters, then press Solve.").arg(solver.rackSize())
                  : (root.result === null
                     ? qsTr("Press Solve to reveal the solution.")
                     : qsTr("No single word in the list uses these letters."))
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
                SolutionContent { anchors.centerIn: parent }
                ResultsEmptyState { anchors.centerIn: parent }
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
                SolutionContent { anchors.centerIn: parent }
                ResultsEmptyState { anchors.centerIn: parent }
            }
        }
    }

    // Phone-landscape - fixed-width tile/keyboard pane split; results stay
    // the same centered content, just resized.
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
                SolutionContent { anchors.centerIn: parent }
                ResultsEmptyState { anchors.centerIn: parent }
            }
        }
    }
}
