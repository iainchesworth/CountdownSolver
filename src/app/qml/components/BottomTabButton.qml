import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Single bottom-tab-bar tab. Vertical (icon-over-label) on tablet-portrait/
// phone-portrait, horizontal (icon-beside-label) on phone-landscape via
// `horizontal`. Active state mirrors NavItem's "always-present, color-only
// toggles" technique (see NavItem.qml's left accent bar) but with a 2px
// top border instead of a 3px left one, since this sits in a horizontal bar.
Button {
    id: control
    property string glyph: ""
    property bool   active: false
    property bool   horizontal: false

    readonly property int idx: Metrics.compactIndex

    flat: true
    hoverEnabled: true
    Layout.fillWidth: true
    Layout.fillHeight: true
    topPadding: horizontal ? 0 : Metrics.tabBar.paddingTop[idx]

    background: Rectangle {
        color: "transparent"
        Rectangle {   // top accent border; anchors (not y) so it mirrors under RTL
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: 2
            color: control.active ? Theme.accent : "transparent"
        }
    }

    contentItem: Loader {
        sourceComponent: control.horizontal ? rowContent : columnContent

        Component {
            id: badgeComponent
            Rectangle {
                width: Metrics.tabBadge.size[control.idx]
                height: Metrics.tabBadge.size[control.idx]
                radius: Metrics.tabBadge.radius[control.idx]
                color: control.active ? Theme.accent : "transparent"
                border.width: control.active ? 0 : 1
                border.color: Theme.tileBorder
                Text {
                    anchors.centerIn: parent
                    text: control.glyph
                    font.family: Theme.mono
                    font.pixelSize: Metrics.tabBadge.font[control.idx]
                    font.weight: Font.DemiBold
                    color: control.active ? Theme.accentInk : Theme.muted
                }
            }
        }
        Component {
            id: labelComponent
            Text {
                text: control.text
                font.family: Theme.sans
                font.pixelSize: Metrics.tabLabel.font[control.idx]
                font.weight: control.active ? Font.DemiBold : Font.Medium
                color: control.active ? Theme.accent : Theme.muted
            }
        }
        Component {
            id: columnContent
            ColumnLayout {
                spacing: Metrics.tabBar.gap[control.idx]
                Loader { sourceComponent: badgeComponent; Layout.alignment: Qt.AlignHCenter }
                Loader { sourceComponent: labelComponent; Layout.alignment: Qt.AlignHCenter }
            }
        }
        Component {
            id: rowContent
            RowLayout {
                spacing: Metrics.tabBar.gap[control.idx]
                Loader { sourceComponent: badgeComponent; Layout.alignment: Qt.AlignVCenter }
                Loader { sourceComponent: labelComponent; Layout.alignment: Qt.AlignVCenter }
            }
        }
    }
}
