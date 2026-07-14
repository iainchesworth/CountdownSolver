import QtQuick

// A game tile. Filled tiles are solid; empty tiles read as a subtle well.
// NOTE: an empty tile in the web design uses a dashed border. QML Rectangle
// borders are solid — for exact parity draw the dashed outline with a Shape
// (Qt Quick Shapes) or a Canvas. Left solid here for simplicity.
Rectangle {
    property string label: ""
    property bool   mono: true
    property int    fontSize: 24
    // True while `label` is a not-yet-committed value being typed on the
    // physical keyboard (e.g. the Numbers buffer before Enter/Space commits it).
    property bool   pending: false
    readonly property bool filled: label.length > 0

    radius: 10
    color: filled ? Theme.tile : Theme.bg
    border.width: filled ? 1 : 2
    border.color: pending ? Theme.accent : Theme.tileBorder

    Text {
        anchors.centerIn: parent
        text: parent.label
        color: parent.pending ? Theme.accent : Theme.tileInk
        font.family: parent.mono ? Theme.mono : Theme.sans
        font.pixelSize: parent.fontSize
        font.weight: Font.DemiBold
        font.capitalization: parent.mono ? Font.MixedCase : Font.AllUppercase
    }
}
