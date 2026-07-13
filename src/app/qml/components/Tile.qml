import QtQuick

// A game tile. Filled tiles are solid; empty tiles read as a subtle well.
// NOTE: an empty tile in the web design uses a dashed border. QML Rectangle
// borders are solid — for exact parity draw the dashed outline with a Shape
// (Qt Quick Shapes) or a Canvas. Left solid here for simplicity.
Rectangle {
    property string label: ""
    property bool   mono: true
    property int    fontSize: 24
    readonly property bool filled: label.length > 0

    radius: 10
    color: filled ? Theme.tile : Theme.bg
    border.width: filled ? 1 : 2
    border.color: Theme.tileBorder

    Text {
        anchors.centerIn: parent
        text: parent.label
        color: Theme.tileInk
        font.family: parent.mono ? Theme.mono : Theme.sans
        font.pixelSize: parent.fontSize
        font.weight: Font.DemiBold
        font.capitalization: parent.mono ? Font.MixedCase : Font.AllUppercase
    }
}
