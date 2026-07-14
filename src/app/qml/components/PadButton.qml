import QtQuick
import QtQuick.Controls

// Grid pad button: number values, digit keys, and keyboard keys.
// accent=true gives the teal-tinted style used for the large numbers (25/50/75/100).
Button {
    id: control
    property bool accent: false
    property bool mono: true
    property int  fontSize: 16

    flat: true
    hoverEnabled: true
    implicitHeight: 44
    implicitWidth: 40
    opacity: control.enabled ? 1.0 : 0.35

    contentItem: Text {
        text: control.text
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        font.family: control.mono ? Theme.mono : Theme.sans
        font.pixelSize: control.fontSize
        font.weight: Font.DemiBold
        font.capitalization: control.mono ? Font.MixedCase : Font.AllUppercase
        color: control.accent ? Theme.accent : Theme.tileInk
    }

    background: Rectangle {
        radius: Theme.radiusControl
        color: control.accent
               ? (control.down ? Qt.darker(Theme.accentSoft, 1.06) : Theme.accentSoft)
               : (control.down ? Qt.darker(Theme.tile, 1.05)
                                : (control.hovered ? Qt.darker(Theme.tile, 1.02) : Theme.tile))
        border.width: 1
        border.color: control.accent ? Theme.accent : Theme.tileBorder
    }
}
