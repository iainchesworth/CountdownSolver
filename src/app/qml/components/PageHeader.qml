import QtQuick
import QtQuick.Layouts

// Phone-landscape's compact top bar: replaces the shared title block Main.qml
// draws for every other form factor (there isn't room for a separate header
// row + toggle bar in 390px of height). Title/subtitle truncate so the
// content-sized ViewToggle on the trailing edge always has room.
Item {
    id: header
    property string title: ""
    property string subtitle: ""
    property bool showToggle: true
    property string currentView: "input"
    property bool hasResult: false
    signal viewActivated(string view)

    implicitHeight: 56

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 18; anchors.rightMargin: 18
        spacing: 16

        ColumnLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: 0
            spacing: 1
            Text {
                Layout.fillWidth: true
                text: header.title
                color: Theme.ink; font.family: Theme.sans
                font.pixelSize: Metrics.headerTitle.font[Metrics.safeIndex]
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            Text {
                Layout.fillWidth: true
                text: header.subtitle
                color: Theme.muted; font.family: Theme.sans
                font.pixelSize: Metrics.headerSubtitle.font[Metrics.safeIndex]
                elide: Text.ElideRight
            }
        }

        ViewToggle {
            Layout.fillWidth: false
            visible: header.showToggle
            currentView: header.currentView
            hasResult: header.hasResult
            onViewActivated: view => header.viewActivated(view)
        }
    }
}
