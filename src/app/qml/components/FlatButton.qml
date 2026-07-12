import QtQuick
import QtQuick.Controls

// Secondary action button. primary=true = accent-outlined (the "Random" action).
Button {
    id: control
    property bool primary: false

    flat: true
    hoverEnabled: true
    implicitHeight: 42
    leftPadding: 16
    rightPadding: 16

    contentItem: Text {
        text: control.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: Theme.sans
        font.pixelSize: 13
        font.weight: Font.DemiBold
        color: control.primary ? Theme.accent : Theme.ink
    }

    background: Rectangle {
        radius: Theme.radiusControl
        color: control.primary
               ? (control.down ? Theme.accentSoft : "transparent")
               : (control.down ? Qt.darker(Theme.panel, 1.05) : Theme.panel)
        border.width: 1
        border.color: control.primary ? Theme.accent : Theme.border
    }
}
