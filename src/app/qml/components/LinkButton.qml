import QtQuick
import QtQuick.Controls

// A text link: accent-coloured label that turns to the ink colour on hover.
// Replaces the bare Text + MouseArea links in Settings and the licences
// dialog, which a keyboard (or screen reader) couldn't reach at all. It is a
// flat Button underneath, so it takes Tab focus, activates with Space/Enter,
// is announced as a link, and shows the shared focus ring.
//
// Set `text` and handle `clicked`, exactly as for a Button.
Button {
    id: control

    flat: true
    hoverEnabled: true
    // Sized exactly like the Text it replaces, so layouts around it don't move.
    padding: 0
    // Tab reaches it; a click doesn't take keyboard focus and never draws the
    // ring (see FlatButton.qml).
    focusPolicy: Qt.TabFocus

    Accessible.role: Accessible.Link
    Accessible.name: control.text

    Keys.onReturnPressed: control.clicked()
    Keys.onEnterPressed: control.clicked()

    background: Item {}
    contentItem: Text {
        text: control.text
        color: control.hovered ? Theme.ink : Theme.accent
        font.family: Theme.sans
        font.pixelSize: 13
        font.weight: Font.DemiBold
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    HoverHandler { cursorShape: Qt.PointingHandCursor }

    FocusRing { controlRadius: 3 }
}
