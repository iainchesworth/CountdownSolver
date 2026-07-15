import QtQuick

// A panel/card surface. Put a ColumnLayout (anchors.fill + margins) inside.
Rectangle {
    property int cornerRadius: Theme.radiusCard
    color: Theme.panel
    border.color: Theme.border
    border.width: 1
    radius: cornerRadius
}
