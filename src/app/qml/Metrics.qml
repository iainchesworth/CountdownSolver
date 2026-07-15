pragma Singleton
import QtQuick

// Per-form-factor sizing tokens for the three new responsive layouts
// (tablet-portrait, phone-portrait, phone-landscape), from RESPONSIVE_SPEC.md
// §5. Desktop and TabletLandscape never read this file - `compactIndex` is
// -1 for both, and every desktop/tablet-landscape layout block keeps its
// existing hardcoded literals untouched, so there's no code path where a
// stray Metrics read could silently produce wrong sizing on those two
// (already shipped) form factors.
//
// `formFactor` is written by Main.qml via a Binding (see Main.qml's
// `formFactor` property) - this singleton never computes it itself, since a
// singleton can't reach out to the ApplicationWindow's width/height.
QtObject {
    id: metrics

    property int formFactor: FormFactor.desktop

    readonly property int compactIndex:
          formFactor === FormFactor.tabletPortrait ? 0
        : formFactor === FormFactor.phonePortrait  ? 1
        : formFactor === FormFactor.phoneLandscape ? 2
        : -1

    // Tappable hit-boxes must stay >=44px on phones even where the visual
    // size in the sizing table dips below that (target keypad keys, some
    // pad buttons) - apply this to a control's implicitWidth/implicitHeight,
    // never to the drawn Rectangle, so the visible size still matches the
    // mockup while the hit area is padded out around it.
    function touchSize(raw) {
        return (formFactor === FormFactor.phonePortrait || formFactor === FormFactor.phoneLandscape)
            ? Math.max(raw, 44)
            : raw
    }

    // ---- sizing table, indexed by compactIndex [tabletPortrait, phonePortrait, phoneLandscape] ----

    readonly property var numberTile:    ({ size: [72, 54, 44], radius: [11, 10, 9], font: [28, 20, 18] })
    readonly property var largePad:      ({ size: [56, 46, 40], radius: [11, 10, 9], font: [20, 16, 15] })
    readonly property var smallPad:      ({ size: [56, 44, 36], radius: [11, 10, 9], font: [19, 16, 14] })
    readonly property var targetDisplay: ({ size: [62, 50, 44], radius: [11, 10, 9], font: [34, 28, 24], letterSpacing: [4, 3, 3] })
    readonly property var targetKey:     ({ size: [52, 38, 32], radius: [9, 7, 6], font: [18, 13, 12] })
    readonly property var actionButton:  ({ size: [56, 48, 42], radius: [11, 10, 9] })
    readonly property var solveButton:   ({ size: [56, 50, 42], radius: [11, 10, 9], font: [17, 16, 15] })
    readonly property var qwertyKey:     ({ size: [60, 46, 42], maxWidth: [72, 34, 46], radius: [10, 8, 8], font: [20, 16, 16] })
    readonly property var rackTile:      ({ size: [68, 50, 44], radius: [11, 10, 9], font: [28, 22, 18] })
    readonly property var conundrumTile: ({ w: [60, 52, 48], h: [70, 62, 56], radius: [11, 10, 9], font: [32, 28, 26] })
    readonly property var cardRadius:    ({ value: [14, 14, 12] })
    readonly property var cardPadding:   ({ value: [18, 16, 14] })
    readonly property var toggleSwitch:  ({ w: [52, 50, 48], h: [30, 28, 28], radius: [15, 14, 14], knob: [24, 22, 22] })
    readonly property var segChip:       ({ h: [36, 34, 32], radius: [7, 7, 6], font: [15, 14, 13] })
    readonly property var headerTitle:   ({ font: [30, 24, 19] })
    readonly property var headerSubtitle: ({ font: [17, 14, 12] })
    // Only used at index 0/1 (tabletPortrait/phonePortrait) - phone-landscape
    // (index 2) uses PageHeader's compact top bar instead of this header
    // block, so its slot here just repeats the phonePortrait value, unused.
    readonly property var headerPadding: ({ top: [26, 22, 22], side: [26, 18, 18], bottom: [16, 12, 12] })
    readonly property var tabBar:        ({ h: [76, 66, 54], gap: [4, 3, 8], paddingTop: [6, 5, 0] })
    readonly property var tabBadge:      ({ size: [30, 28, 26], radius: [8, 8, 7], font: [13, 12, 12] })
    readonly property var tabLabel:      ({ font: [13, 11, 13] })

    // The Input/Results toggle has its own sizing group - unlike segChip
    // (Settings, label-sized chips), it's either full-width-split (portrait
    // forms, no horizontal padding needed) or content-sized with fixed
    // side-padding (phone-landscape, inline in the compact top bar).
    readonly property var viewToggleBar: ({ radius: [12, 11, 10] })
    readonly property var viewToggle:    ({ h: [46, 40, 34], radius: [9, 8, 7], font: [16, 14, 13], hPad: [0, 0, 18] })
}
