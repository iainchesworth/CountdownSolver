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
    opacity: control.enabled ? 1.0 : 0.45

    // Reachable with Tab, but a mouse click leaves keyboard focus where it was
    // (usually the page, which turns typed digits/letters into game input -
    // see NumbersPage.qml's Keys.onPressed). Only keyboard focus draws a ring.
    focusPolicy: Qt.TabFocus

    // Name is the visible label; override it where the label is a glyph
    // ("←" is "Backspace").
    Accessible.role: Accessible.Button
    Accessible.name: control.text

    // Space is Button's own; Enter/Return activate it the same way, rather
    // than falling through to the page's Enter-means-Solve shortcut.
    Keys.onReturnPressed: control.clicked()
    Keys.onEnterPressed: control.clicked()

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

    FocusRing { controlRadius: Theme.radiusControl }
}
