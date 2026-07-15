import QtQuick
import QtQuick.Layouts

// Bottom navigation for tablet-portrait/phone-portrait/phone-landscape,
// replacing Main.qml's sidebar on those form factors. `tabs` takes the same
// [[title, glyph], ...] shape the sidebar's NavItem Repeater already uses
// (see Main.qml) so both navigation UIs are built from one source list.
Rectangle {
    id: root
    property var tabs: []
    property int currentIndex: 0
    signal tabActivated(int index)

    // safeIndex, not compactIndex - see BottomTabButton.qml's comment.
    implicitHeight: Metrics.tabBar.h[Metrics.safeIndex]
    color: Theme.sidebar

    Rectangle { anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; height: 1; color: Theme.border }

    RowLayout {
        anchors.fill: parent
        spacing: 0
        Repeater {
            model: root.tabs
            delegate: BottomTabButton {
                text: modelData[0]
                glyph: modelData[1]
                active: root.currentIndex === index
                horizontal: Metrics.formFactor === FormFactor.phoneLandscape
                onClicked: root.tabActivated(index)
            }
        }
    }
}
