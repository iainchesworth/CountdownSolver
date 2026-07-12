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
            width: Math.min(parent.width, 620)
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
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: "Theme"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                            Text { text: "Applies across the whole window."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                        }
                        SegControl {
                            options: ["Light", "Dark"]
                            currentIndex: Theme.dark ? 1 : 0
                            onActivated: Theme.dark = (index === 1)
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

                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: "Minimum word length"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                            Text { text: "Shortest words shown in the letters game."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                        }
                        SegControl {
                            options: ["3+", "4+", "5+"]
                            currentIndex: AppState.minLen - 3
                            onActivated: AppState.minLen = index + 3
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { text: "Flag when no exact answer"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                            Text { text: "Highlight the closest result in the numbers game."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                        }
                        Switch {
                            checked: AppState.flagInexact
                            onToggled: AppState.flagInexact = checked
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 12
                        RowLayout {
                            Layout.fillWidth: true
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Text { text: "Max words shown"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                Text { text: "Cap the letters-game result list."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                            }
                            Text { text: AppState.maxResults; color: Theme.accent; font.family: Theme.mono; font.pixelSize: 16; font.weight: Font.DemiBold }
                        }
                        Slider {
                            Layout.fillWidth: true
                            from: 20; to: 150; stepSize: 10
                            value: AppState.maxResults
                            onMoved: AppState.maxResults = value
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
                            Text { text: "Swap the sample list for a full tournament list."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                        }
                        SegControl {
                            options: ["Sample", "SOWPODS"]
                            currentIndex: 0
                            // TODO: wire to your dictionary loader; SOWPODS disabled until bundled.
                            onActivated: currentIndex = 0
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
                    Text {
                        Layout.fillWidth: true
                        text: solver.versionDetails()
                        color: Theme.muted
                        font.family: Theme.mono
                        font.pixelSize: 13
                        wrapMode: Text.WordWrap
                    }
                }
            }

            Item { Layout.preferredHeight: 4 }
        }
    }
}
