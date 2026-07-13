import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Sidebar navigation item: glyph badge + label, active state with left accent bar.
Button {
    id: control
    property string glyph: ""
    property bool   active: false

    flat: true
    hoverEnabled: true
    implicitHeight: 44

    background: Rectangle {
        radius: Theme.radiusControl
        color: control.active ? Theme.accentSoft
                              : (control.hovered ? Qt.rgba(0, 0, 0, 0.03) : "transparent")
        Rectangle {   // left accent bar
            width: 3
            height: parent.height
            radius: 2
            color: control.active ? Theme.accent : "transparent"
        }
    }

    contentItem: RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        spacing: 11
        Rectangle {
            Layout.preferredWidth: 26
            Layout.preferredHeight: 26
            radius: 7
            color: control.active ? Theme.accent : Theme.tile
            border.width: control.active ? 0 : 1
            border.color: Theme.border
            Text {
                anchors.centerIn: parent
                text: control.glyph
                font.family: Theme.mono
                font.pixelSize: 12
                font.weight: Font.DemiBold
                color: control.active ? Theme.accentInk : Theme.muted
            }
        }
        Text {
            Layout.fillWidth: true
            text: control.text
            font.family: Theme.sans
            font.pixelSize: 14
            font.weight: control.active ? Font.DemiBold : Font.Medium
            color: control.active ? Theme.accent : Theme.ink
        }
    }
}
