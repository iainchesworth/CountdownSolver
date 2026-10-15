import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Third-party licence attributions, opened from the About card in Settings.
// Add an entry to `entries` for each library bundled in the shipped app.
//
// Everything a person reads here is translated (qsTr), except what is a
// proper name rather than prose: library and font names, and the licences'
// own titles ("SIL Open Font License, Version 1.1" is the title of a legal
// text, and is cited in English whatever the UI language).
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
            summary: qsTr("Application framework and GUI toolkit (Core, Gui, Quick, QuickControls2, Concurrent)."),
            licenseName: "GNU Lesser General Public License v3.0",
            licenseUrl: "https://www.gnu.org/licenses/lgpl-3.0.html"
        },
        {
            name: "IBM Plex Sans",
            version: "",
            summary: qsTr("Bundled font used for the app's default (Latin-script) UI typeface."),
            licenseName: "SIL Open Font License, Version 1.1",
            licenseUrl: "https://scripts.sil.org/OFL"
        },
        {
            name: "IBM Plex Mono",
            version: "",
            summary: qsTr("Bundled font used for numbers, code-style text, and version/meta labels."),
            licenseName: "SIL Open Font License, Version 1.1",
            licenseUrl: "https://scripts.sil.org/OFL"
        },
        {
            name: "Noto Sans Arabic",
            version: "",
            summary: qsTr("Bundled font providing Arabic script coverage for the Arabic display language."),
            licenseName: "SIL Open Font License, Version 1.1",
            licenseUrl: "https://scripts.sil.org/OFL"
        },
        {
            name: "Noto Sans Hebrew",
            version: "",
            summary: qsTr("Bundled font providing Hebrew script coverage for the Hebrew and Yiddish display languages."),
            licenseName: "SIL Open Font License, Version 1.1",
            licenseUrl: "https://scripts.sil.org/OFL"
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
            Text { text: qsTr("Third-party licences"); color: Theme.ink; font.family: Theme.sans; font.pixelSize: 17; font.weight: Font.Bold }
            Text { text: qsTr("Libraries bundled with Countdown Solver."); color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
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
                LinkButton {
                    text: modelData.licenseName + " ↗"
                    onClicked: Qt.openUrlExternally(modelData.licenseUrl)
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
            text: qsTr("Close")
            onClicked: root.close()
        }
    }
}
