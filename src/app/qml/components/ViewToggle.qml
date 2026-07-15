import QtQuick

// Input/Results toggle for the three new form factors' game pages (hidden
// on Settings - `showToggle = tab !== 'settings'` is the caller's concern,
// not this component's). Full-width split on tablet-portrait/phone-portrait
// (own bar under the header); content-sized on phone-landscape (inline in
// PageHeader's compact top bar, right-aligned next to the title).
SegControl {
    id: toggle
    property string currentView: "input"
    property bool hasResult: false
    signal viewActivated(string view)

    readonly property int idx: Metrics.compactIndex
    readonly property bool compact: Metrics.formFactor === FormFactor.phoneLandscape

    options: [qsTr("Input"), hasResult ? qsTr("Results") + " •" : qsTr("Results")]
    currentIndex: currentView === "results" ? 1 : 0
    stretch: !compact
    barColor: Theme.sidebar
    barRadius: Metrics.viewToggleBar.radius[idx]
    segHeight: Metrics.viewToggle.h[idx]
    segRadius: Metrics.viewToggle.radius[idx]
    pixelSize: Metrics.viewToggle.font[idx]
    hPadding: Metrics.viewToggle.hPad[idx]

    onActivated: index => toggle.viewActivated(index === 0 ? "input" : "results")
}
