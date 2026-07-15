pragma Singleton
import QtQuick

// Named constants for Main.qml's `formFactor` property. Desktop and
// TabletLandscape are the shipped sidebar layout (unchanged by the
// responsive work); the other three get a bottom tab bar + new per-page
// layouts - see Metrics.qml for their sizing tables.
QtObject {
    readonly property int desktop: 0
    readonly property int tabletLandscape: 1
    readonly property int tabletPortrait: 2
    readonly property int phonePortrait: 3
    readonly property int phoneLandscape: 4
}
