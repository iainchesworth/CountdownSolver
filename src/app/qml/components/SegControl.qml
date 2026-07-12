import QtQuick

// Segmented control (used in Settings). Set `options` to a list of labels;
// listen to activated(index). Bind currentIndex to your state.
Rectangle {
    id: seg
    property var options: []
    property int currentIndex: 0
    signal activated(int index)

    implicitHeight: 40
    implicitWidth: row.width + 8
    radius: Theme.radiusControl
    color: Theme.bg
    border.width: 1
    border.color: Theme.border

    Row {
        id: row
        anchors.centerIn: parent
        spacing: 6
        Repeater {
            model: seg.options
            delegate: Rectangle {
                height: 32
                width: label.implicitWidth + 28
                radius: 6
                color: index === seg.currentIndex ? Theme.accent : "transparent"
                Text {
                    id: label
                    anchors.centerIn: parent
                    text: modelData
                    font.family: Theme.sans
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                    color: index === seg.currentIndex ? Theme.accentInk : Theme.muted
                }
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        seg.currentIndex = index
                        seg.activated(index)
                    }
                }
            }
        }
    }
}
