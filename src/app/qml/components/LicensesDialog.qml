import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Third-party license attributions, opened from the About card in Settings.
// Add an entry to `entries` for each library bundled in the shipped app.
Dialog {
    id: root
    modal: true
    focus: true
    padding: 0
    closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
    anchors.centerIn: parent
    width: Math.min(560, (parent ? parent.width : 560) - 80)
    height: Math.min(460, (parent ? parent.height : 460) - 80)

    readonly property var entries: [
        {
            name: "Qt 6",
            version: solver.qtVersion(),
            summary: "Application framework and GUI toolkit (Core, Gui, Quick, QuickControls2, Concurrent).",
            licenseName: "GNU Lesser General Public License v3.0",
            licenseUrl: "https://www.gnu.org/licenses/lgpl-3.0.html"
        }
    ]

    background: Rectangle {
        color: Theme.panel
        border.color: Theme.border
        border.width: 1
        radius: Theme.radiusCard
    }

    header: Item {
        implicitHeight: headerCol.implicitHeight + 32
        ColumnLayout {
            id: headerCol
            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top
            anchors.margins: 20
            spacing: 4
            Text { text: "Third-Party Licenses"; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 17; font.weight: Font.Bold }
            Text { text: "Libraries bundled with Countdown Solver."; color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
        }
        Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom; height: 1; color: Theme.border }
    }

    contentItem: ListView {
        id: list
        clip: true
        model: root.entries
        boundsBehavior: Flickable.StopAtBounds
        ScrollBar.vertical: ScrollBar {}

        delegate: Item {
            width: list.width
            implicitHeight: entryCol.implicitHeight + 24

            ColumnLayout {
                id: entryCol
                x: 20; y: 12
                width: parent.width - 40
                spacing: 4

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8
                    Text { text: modelData.name; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                    Text { text: modelData.version ? ("v" + modelData.version) : ""; color: Theme.faint; font.family: Theme.mono; font.pixelSize: 12 }
                }
                Text {
                    Layout.fillWidth: true
                    text: modelData.summary
                    color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13
                    wrapMode: Text.WordWrap
                }
                Text {
                    id: licenseLink
                    text: modelData.licenseName + " ↗"
                    color: licenseLinkArea.containsMouse ? Theme.ink : Theme.accent
                    font.family: Theme.sans; font.pixelSize: 13; font.weight: Font.DemiBold
                    MouseArea {
                        id: licenseLinkArea
                        anchors.fill: parent
                        hoverEnabled: true
                        cursorShape: Qt.PointingHandCursor
                        onClicked: Qt.openUrlExternally(modelData.licenseUrl)
                    }
                }
            }

            Rectangle {
                visible: index < list.count - 1
                anchors.left: parent.left; anchors.right: parent.right; anchors.bottom: parent.bottom
                anchors.leftMargin: 20; anchors.rightMargin: 20
                height: 1
                color: Theme.border
            }
        }
    }

    footer: Item {
        implicitHeight: 60
        Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; height: 1; color: Theme.border }
        FlatButton {
            anchors.right: parent.right; anchors.rightMargin: 20
            anchors.verticalCenter: parent.verticalCenter
            text: "Close"
            onClicked: root.close()
        }
    }
}
