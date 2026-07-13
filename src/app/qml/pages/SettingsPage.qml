import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Settings. Writes straight into the Theme (visual) and AppState (solver) singletons.
Item {
    id: root

    Flickable {
        anchors.fill: parent
        anchors.leftMargin: 24; anchors.rightMargin: 24; anchors.bottomMargin: 24
        contentHeight: col.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: col
            width: parent.width
            spacing: 16

            // ---- Appearance ----
            Card {
                Layout.fillWidth: true
                implicitHeight: ap.implicitHeight + 40
                ColumnLayout {
                    id: ap
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 16
                    SectionLabel { text: "Appearance" }
                    Item {
                        Layout.fillWidth: true
                        implicitHeight: themeRow.implicitHeight
                        RowLayout {
                            id: themeRow
                            anchors.fill: parent
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Text { text: "Theme"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                Text { text: "Applies across the whole window."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                            }
                            SegControl {
                                Layout.fillWidth: false
                                options: ["Light", "Dark"]
                                currentIndex: Theme.dark ? 1 : 0
                                onActivated: Theme.dark = (index === 1)
                            }
                        }
                    }
                }
            }

            // ---- Solver ----
            Card {
                Layout.fillWidth: true
                implicitHeight: so.implicitHeight + 40
                ColumnLayout {
                    id: so
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 20
                    SectionLabel { text: "Solver" }

                    Item {
                        Layout.fillWidth: true
                        implicitHeight: minLenRow.implicitHeight
                        RowLayout {
                            id: minLenRow
                            anchors.fill: parent
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Text { text: "Minimum word length"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                Text { text: "Shortest words shown in the letters game."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                            }
                            SegControl {
                                Layout.fillWidth: false
                                options: ["3+", "4+", "5+"]
                                currentIndex: AppState.minLen - 3
                                onActivated: AppState.minLen = index + 3
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        implicitHeight: flagRow.implicitHeight
                        RowLayout {
                            id: flagRow
                            anchors.fill: parent
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Text { text: "Flag when no exact answer"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                Text { text: "Highlight the closest result in the numbers game."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                            }
                            Switch {
                                id: flagSwitch
                                Layout.fillWidth: false
                                checked: AppState.flagInexact
                                onToggled: AppState.flagInexact = checked
                                background: Item {}
                                indicator: Rectangle {
                                    implicitWidth: 40; implicitHeight: 22
                                    radius: 11
                                    color: flagSwitch.checked ? Theme.accent : Theme.bg
                                    border.width: flagSwitch.checked ? 0 : 1
                                    border.color: Theme.tileBorder
                                    Rectangle {
                                        width: 16; height: 16; radius: 8
                                        anchors.verticalCenter: parent.verticalCenter
                                        x: flagSwitch.checked ? parent.width - width - 3 : 3
                                        color: flagSwitch.checked ? Theme.accentInk : Theme.faint
                                        Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.InOutQuad } }
                                    }
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 12
                        Item {
                            Layout.fillWidth: true
                            implicitHeight: maxWordsRow.implicitHeight
                            RowLayout {
                                id: maxWordsRow
                                anchors.fill: parent
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 2
                                    Text { text: "Max words shown"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                    Text { text: "Cap the letters-game result list."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                                }
                                Text { text: AppState.maxResults; color: Theme.accent; font.family: Theme.mono; font.pixelSize: 16; font.weight: Font.DemiBold }
                            }
                        }
                        Slider {
                            id: wordsSlider
                            Layout.fillWidth: true
                            from: 20; to: 150; stepSize: 10
                            value: AppState.maxResults
                            onMoved: AppState.maxResults = value
                            background: Rectangle {
                                x: wordsSlider.leftPadding
                                y: wordsSlider.topPadding + wordsSlider.availableHeight / 2 - height / 2
                                width: wordsSlider.availableWidth; height: 4; radius: 2
                                color: Theme.border
                                Rectangle {
                                    width: wordsSlider.visualPosition * parent.width
                                    height: parent.height; radius: 2
                                    color: Theme.accent
                                }
                            }
                            handle: Rectangle {
                                x: wordsSlider.leftPadding + wordsSlider.visualPosition * (wordsSlider.availableWidth - width)
                                y: wordsSlider.topPadding + wordsSlider.availableHeight / 2 - height / 2
                                width: 14; height: 14; radius: 3
                                color: Theme.accent
                            }
                        }
                    }
                }
            }

            // ---- Dictionary ----
            Card {
                Layout.fillWidth: true
                implicitHeight: di.implicitHeight + 40
                ColumnLayout {
                    id: di
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 16
                    SectionLabel { text: "Dictionary" }
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: "Word list"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                            Text {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                text: solver.fullDictionaryAvailable()
                                      ? "Swap the built-in dictionary for your custom words.txt list."
                                      : solver.fullDictionaryStatus()
                                color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13
                            }
                        }
                        SegControl {
                            id: dictSeg
                            objectName: "dictSeg"
                            Layout.fillWidth: false
                            options: ["Default", "Custom"]
                            enabled: solver.fullDictionaryAvailable()
                            opacity: enabled ? 1 : 0.5
                            currentIndex: AppState.useFullDictionary ? 1 : 0
                            onActivated: {
                                const wantFull = index === 1
                                if (solver.setUseFullDictionary(wantFull)) {
                                    AppState.useFullDictionary = wantFull
                                } else {
                                    dictSeg.currentIndex = AppState.useFullDictionary ? 1 : 0
                                }
                            }
                        }
                    }
                }
            }

            // ---- About ----
            Card {
                Layout.fillWidth: true
                implicitHeight: ab.implicitHeight + 40
                ColumnLayout {
                    id: ab
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 12
                    SectionLabel { text: "About" }
                    ColumnLayout {
                        id: aboutText
                        objectName: "aboutText"
                        Layout.fillWidth: true
                        spacing: 4
                        readonly property var versionLines: solver.versionDetails().split("\n")
                        Text {
                            Layout.fillWidth: true
                            text: aboutText.versionLines[0] || ""
                            color: Theme.ink; font.family: Theme.sans
                            font.pixelSize: 15; font.weight: Font.DemiBold
                        }
                        Text {
                            Layout.fillWidth: true
                            visible: text.length > 0
                            text: aboutText.versionLines.slice(1).join("\n")
                            color: Theme.muted; font.family: Theme.mono
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                        }
                    }
                    Text {
                        id: repoLink
                        Layout.topMargin: 4
                        text: "View on GitHub ↗"
                        color: repoLinkArea.containsMouse ? Theme.ink : Theme.accent
                        font.family: Theme.sans; font.pixelSize: 13; font.weight: Font.DemiBold
                        MouseArea {
                            id: repoLinkArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally("https://github.com/iainchesworth/CountdownSolver")
                        }
                    }
                }
            }

            Item { Layout.preferredHeight: 4 }
        }
    }
}
