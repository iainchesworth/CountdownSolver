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
    topPadding: 0
    bottomPadding: 0
    // Leading-edge inset via padding (not an anchor margin on contentItem) so
    // Control's own padding-driven content geometry is the only thing sizing
    // contentItem - no competing anchors.fill binding to race against it.
    // `mirrored` is Control's built-in RTL flag, so this still flips correctly.
    leftPadding: control.mirrored ? 0 : 12
    rightPadding: control.mirrored ? 12 : 0

    background: Rectangle {
        radius: Theme.radiusControl
        color: control.active ? Theme.accentSoft
                              : (control.hovered ? Qt.rgba(0, 0, 0, 0.03) : "transparent")
        Rectangle {   // leading accent bar; anchors (not x) so it mirrors under RTL
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            width: 3
            radius: 2
            color: control.active ? Theme.accent : "transparent"
        }
    }

    contentItem: RowLayout {
        spacing: 11
        Rectangle {
            // Fixed min/max so a long label in a wide script (e.g. Yiddish
            // "Settings") can never squeeze this badge - RowLayout shrinks
            // *every* child proportionally once total preferred width
            // overflows, and preferredWidth alone doesn't stop that.
            Layout.minimumWidth: 26
            Layout.preferredWidth: 26
            Layout.maximumWidth: 26
            Layout.minimumHeight: 26
            Layout.preferredHeight: 26
            Layout.maximumHeight: 26
            Layout.alignment: Qt.AlignVCenter
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
            Layout.minimumWidth: 0
            Layout.alignment: Qt.AlignVCenter
            text: control.text
            font.family: Theme.sans
            font.pixelSize: 14
            font.weight: control.active ? Font.DemiBold : Font.Medium
            color: control.active ? Theme.accent : Theme.ink
            elide: Text.ElideRight
        }
    }
}
