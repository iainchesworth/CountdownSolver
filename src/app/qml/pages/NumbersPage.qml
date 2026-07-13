import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Numbers game. Local UI state (selection + target); solving is delegated to
// the C++ `solver` context property. See Solver.h for the return contract.
// Solving only happens when the user presses Solve (or Enter) - entering
// numbers/target never solves implicitly.
Item {
    id: root
    focus: StackLayout.isCurrentItem
    property var    numbers: []
    property string target: ""
    property var    result: null   // { value, diff, exact, steps[] }
    // Digits typed on the physical keyboard, buffered until Enter/Space
    // commits them as one of the six numbers (values can be 1-3 digits, so a
    // single keypress can't be committed unambiguously on its own).
    property string numberBuffer: ""
    readonly property var legalNumbers: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 25, 50, 75, 100]
    // The real board: one tile each of 25/50/75/100, two tiles each of 1-10.
    readonly property var poolSupply: ({
        "25": 1, "50": 1, "75": 1, "100": 1,
        "1": 2, "2": 2, "3": 2, "4": 2, "5": 2, "6": 2, "7": 2, "8": 2, "9": 2, "10": 2
    })
    function remainingOf(v) {
        const used = numbers.filter(function (n) { return n === v }).length
        return poolSupply[String(v)] - used
    }
    // The real target generator only ever produces a 3-digit number from 100-999.
    readonly property var validTargetRegex: /^[1-9][0-9]{2}$/
    function targetIsValid() { return validTargetRegex.test(target) }

    function recalc() {
        if (typeof solver === "undefined" || !solver) { result = null; return }
        if (numbers.length === 6 && targetIsValid())
            result = solver.solveNumbers(numbers, parseInt(target))
        else
            result = null
    }
    function addNumber(v)   { if (numbers.length < 6 && remainingOf(v) > 0) { numbers = numbers.concat([v]) } }
    function targetDigit(d) { if (target.length < 3) { target = target + d } }
    function commitBuffer() {
        if (numberBuffer.length === 0 || numbers.length >= 6) return
        const v = parseInt(numberBuffer)
        if (legalNumbers.indexOf(v) !== -1 && remainingOf(v) > 0) numbers = numbers.concat([v])
        numberBuffer = ""
    }
    // Routes a typed digit to the number buffer while numbers are still being
    // chosen, then to the target once all six are in.
    function typeDigit(d) {
        if (numbers.length < 6) { if (numberBuffer.length < 3) numberBuffer = numberBuffer + d }
        else targetDigit(d)
    }
    // Context-aware backspace: clears a pending typed digit first, then the
    // last committed number, then falls back to editing the target once all
    // six numbers are set.
    function backspaceKey() {
        if (numbers.length < 6) {
            if (numberBuffer.length > 0) numberBuffer = numberBuffer.slice(0, -1)
            else numbers = numbers.slice(0, -1)
        } else if (target.length > 0) {
            target = target.slice(0, -1)
        } else {
            numbers = numbers.slice(0, -1)
        }
    }
    function clearAll()     { numbers = []; target = ""; numberBuffer = ""; result = null }
    function randomGame() {
        var large = [25, 50, 75, 100]
        var big = Math.floor(Math.random() * 5)
        var pool = large.slice().sort(function () { return Math.random() - 0.5 }).slice(0, big)
        var smalls = []
        for (var n = 1; n <= 10; n++) { smalls.push(n); smalls.push(n) }
        smalls.sort(function () { return Math.random() - 0.5 })
        while (pool.length < 6) pool.push(smalls.pop())
        pool.sort(function () { return Math.random() - 0.5 })
        numbers = pool
        target = String(101 + Math.floor(Math.random() * 899))
        numberBuffer = ""
        recalc()
    }

    Keys.onPressed: function (event) {
        if (event.key >= Qt.Key_0 && event.key <= Qt.Key_9) {
            root.typeDigit(String.fromCharCode(event.key))
            event.accepted = true
        } else if (event.key === Qt.Key_Backspace) {
            root.backspaceKey()
            event.accepted = true
        } else if (event.key === Qt.Key_Space && numbers.length < 6) {
            root.commitBuffer()
            event.accepted = true
        } else if (event.key === Qt.Key_Enter || event.key === Qt.Key_Return) {
            if (numbers.length < 6) root.commitBuffer()
            else root.recalc()
            event.accepted = true
        } else if (event.key === Qt.Key_Escape) {
            root.clearAll()
            event.accepted = true
        }
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
                implicitHeight: selCol.implicitHeight + 36
                ColumnLayout {
                    id: selCol
                    anchors.fill: parent; anchors.margins: 18
                    spacing: 12
                    SectionLabel { text: "Your six numbers" }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 8
                        Repeater {
                            model: 6
                            delegate: Tile {
                                Layout.fillWidth: true
                                Layout.preferredHeight: 62
                                label: index < root.numbers.length ? String(root.numbers[index])
                                       : (index === root.numbers.length ? root.numberBuffer : "")
                                pending: index === root.numbers.length && root.numberBuffer.length > 0
                            }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true; spacing: 8
                        Repeater {
                            model: [25, 50, 75, 100]
                            delegate: PadButton {
                                Layout.fillWidth: true; implicitHeight: 46; fontSize: 17
                                accent: true; text: String(modelData)
                                enabled: root.remainingOf(modelData) > 0 && root.numbers.length < 6
                                onClicked: root.addNumber(modelData)
                            }
                        }
                    }
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 5; rowSpacing: 8; columnSpacing: 8
                        Repeater {
                            model: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
                            delegate: PadButton {
                                Layout.fillWidth: true; implicitHeight: 42
                                text: String(modelData)
                                enabled: root.remainingOf(modelData) > 0 && root.numbers.length < 6
                                onClicked: root.addNumber(modelData)
                            }
                        }
                    }
                }
            }

            Card {
                Layout.fillWidth: true
                implicitHeight: tgtCol.implicitHeight + 36
                ColumnLayout {
                    id: tgtCol
                    anchors.fill: parent; anchors.margins: 18
                    spacing: 14
                    RowLayout {
                        Layout.fillWidth: true; spacing: 16
                        SectionLabel { text: "Target" }
                        Rectangle {
                            Layout.fillWidth: true; Layout.preferredHeight: 52
                            radius: 10; color: Theme.accent
                            Text {
                                anchors.centerIn: parent; text: root.target
                                color: Theme.accentInk; font.family: Theme.mono
                                font.pixelSize: 28; font.weight: Font.DemiBold; font.letterSpacing: 3
                            }
                        }
                    }
                    GridLayout {
                        Layout.fillWidth: true
                        columns: 10; columnSpacing: 6; rowSpacing: 6
                        Repeater {
                            model: [1, 2, 3, 4, 5, 6, 7, 8, 9, 0]
                            delegate: PadButton {
                                Layout.fillWidth: true; implicitHeight: 38; fontSize: 14
                                text: String(modelData)
                                onClicked: root.targetDigit(String(modelData))
                            }
                        }
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true; spacing: 9
                FlatButton { Layout.fillWidth: true; text: "\u21bb Random game"; onClicked: root.randomGame() }
                FlatButton { text: "\u232b"; onClicked: root.backspaceKey() }
                FlatButton { text: "Clear"; onClicked: root.clearAll() }
            }
            FlatButton {
                Layout.fillWidth: true
                primary: true
                text: "Solve"
                enabled: root.numbers.length === 6 && root.targetIsValid()
                onClicked: root.recalc()
            }
            Item { Layout.fillHeight: true }
        }

        // ---- right: result ----
        Card {
            Layout.fillWidth: true
            Layout.fillHeight: true

            ColumnLayout {
                anchors.fill: parent; anchors.margins: 22
                spacing: 0
                visible: root.result !== null

                RowLayout {
                    Layout.fillWidth: true
                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 8
                        SectionLabel { text: "Best result" }
                        Text {
                            text: root.result ? String(root.result.value) : ""
                            color: Theme.ink; font.family: Theme.mono
                            font.pixelSize: 46; font.weight: Font.DemiBold
                        }
                    }
                    Rectangle {
                        Layout.alignment: Qt.AlignRight | Qt.AlignVCenter
                        radius: 20
                        implicitHeight: 30; implicitWidth: badge.implicitWidth + 28
                        color: (root.result && root.result.exact) ? Theme.accent
                               : (AppState.flagInexact ? Theme.warnBg : Theme.bg)
                        border.width: (root.result && root.result.exact) ? 0 : 1
                        border.color: Theme.border
                        Text {
                            id: badge
                            anchors.centerIn: parent
                            text: root.result ? (root.result.exact ? "Exact" : root.result.diff + " away") : ""
                            font.family: Theme.sans; font.pixelSize: 13; font.weight: Font.DemiBold
                            color: (root.result && root.result.exact) ? Theme.accentInk
                                   : (AppState.flagInexact ? Theme.warnInk : Theme.muted)
                        }
                    }
                }

                Rectangle { Layout.fillWidth: true; Layout.topMargin: 16; Layout.bottomMargin: 16; height: 1; color: Theme.border }
                SectionLabel { text: "Working" }
                ColumnLayout {
                    Layout.fillWidth: true; Layout.topMargin: 10; spacing: 7
                    Repeater {
                        model: root.result ? root.result.steps : []
                        delegate: Rectangle {
                            Layout.fillWidth: true
                            implicitHeight: 44
                            radius: 9; color: Theme.bg; border.width: 1; border.color: Theme.border
                            RowLayout {
                                anchors.fill: parent; anchors.leftMargin: 14; spacing: 12
                                Rectangle {
                                    width: 22; height: 22; radius: 6; color: Theme.accentSoft
                                    Text { anchors.centerIn: parent; text: index + 1; color: Theme.accent; font.family: Theme.mono; font.pixelSize: 12; font.weight: Font.DemiBold }
                                }
                                Text { Layout.fillWidth: true; text: modelData; color: Theme.ink; font.family: Theme.mono; font.pixelSize: 18; font.weight: Font.Medium }
                            }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
            }

            // empty state
            ColumnLayout {
                anchors.centerIn: parent
                visible: root.result === null
                spacing: 10
                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    width: 52; height: 52; radius: 12; color: "transparent"
                    border.width: 2; border.color: Theme.tileBorder
                    Text { anchors.centerIn: parent; text: "="; color: Theme.faint; font.family: Theme.mono; font.pixelSize: 20; font.weight: Font.DemiBold }
                }
                Text {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: 230
                    horizontalAlignment: Text.AlignHCenter
                    wrapMode: Text.WordWrap
                    text: root.numbers.length < 6
                          ? "Choose " + (6 - root.numbers.length) + " more number" + (root.numbers.length === 5 ? "" : "s") + ", then set a target."
                          : (root.target.length < 3
                             ? "Enter a 3-digit target (100–999), then press Solve."
                             : (!root.targetIsValid()
                                ? "Target must be between 100 and 999."
                                : "Press Solve to see the best result."))
                    color: Theme.muted; font.family: Theme.sans; font.pixelSize: 15
                }
            }
        }
    }
}
