import QtQuick
import QtQuick.Layouts

// Segmented control (used in Settings, and as the Input/Results toggle in
// ViewToggle). Set `options` to a list of labels; listen to activated(index).
// Bind currentIndex to your state.
//
// `stretch: true` splits the full width evenly across segments (ViewToggle
// on tablet-portrait/phone-portrait); the default (content-sized segments,
// centered) is what Settings' existing usages keep. The size/font override
// props all default to today's literals so every existing call site is
// unaffected unless it opts in.
Rectangle {
    id: seg
    property var options: []
    property int currentIndex: 0
    property bool stretch: false
    property int segHeight: 32
    property int segRadius: 6
    property int pixelSize: 13
    property int hPadding: 14
    property color barColor: Theme.bg
    property color barBorderColor: Theme.border
    property int barRadius: Theme.radiusControl
    signal activated(int index)

    implicitHeight: segHeight + 8
    implicitWidth: stretch ? 0 : row.width + 8
    radius: barRadius
    color: barColor
    border.width: 1
    border.color: barBorderColor

    Row {
        id: row
        visible: !seg.stretch
        anchors.centerIn: parent
        spacing: 6
        Repeater {
            // The array itself, not options.length - see SegChip.qml for
            // why: modelData/index as implicit Repeater context properties
            // work correctly, but a `required property` fed an array index
            // at delegate instantiation does not, under this project's AOT
            // QML compilation.
            model: seg.stretch ? [] : seg.options
            delegate: SegChip { owner: seg }
        }
    }

    RowLayout {
        anchors.fill: parent
        anchors.margins: 4
        spacing: 6
        visible: seg.stretch
        Repeater {
            model: seg.stretch ? seg.options : []
            delegate: SegChip { owner: seg }
        }
    }
}
