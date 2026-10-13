import QtQuick

// Keyboard-focus indicator shared by every interactive custom control: a
// Theme.focusRingWidth-px line in the accent colour, drawn Theme.focusRingOffset
// px outside the control's edge (Theme.focusRingOutset px in total). Declare
// one as a child of the control:
//
//     Button { ... FocusRing { controlRadius: Theme.radiusControl } }
//
// Shown for *keyboard* focus only. `visualFocus` is Control's own flag for
// "has focus, and it got there via Tab/Backtab/a shortcut" - it stays false
// for a mouse click or a touch, so tapping a button never leaves a ring behind.
//
// Because Tab can land on a control that is scrolled out of view, a ring that
// appears also scrolls the nearest Flickable/ListView ancestor just far enough
// to bring itself fully into view.
Rectangle {
    id: ring

    // Anything exposing Control's `visualFocus` (Button, Switch, Slider,
    // ComboBox, ...). Defaults to the item the ring is declared in. A `var`,
    // not an `Item`: visualFocus belongs to Control, not QQuickItem.
    property var control: parent
    // The control's own corner radius, so the ring hugs its shape.
    property real controlRadius: 0
    // Draw inside the control's edge instead of outside it. Only for a control
    // that sits flush against the edge of the window (the bottom tab bar),
    // where an outside ring would be cut off.
    property bool inside: false

    anchors.fill: parent
    anchors.margins: inside ? Theme.focusRingOffset : -Theme.focusRingOutset
    radius: inside ? Math.max(0, controlRadius - Theme.focusRingOffset)
                   : controlRadius + Theme.focusRingOutset
    color: "transparent"
    border.width: Theme.focusRingWidth
    border.color: Theme.accent
    visible: !!control && control.visualFocus === true
    // Above the control's own content and any later siblings.
    z: 1000

    onVisibleChanged: {
        if (visible) {
            scrollIntoView()
        }
    }

    // A focused control that becomes disabled (a used-up number pad, Solve
    // while a solve is running) silently loses active focus, which would
    // leave Tab and typed keys restarting from the top of the window. Hand
    // focus to the next control in the chain instead - where Tab would have
    // taken the user next anyway. `focus` (not `activeFocus`) is what's
    // checked: Qt clears the latter first but leaves the former set.
    Connections {
        target: ring.control
        ignoreUnknownSignals: true
        function onEnabledChanged() {
            const c = ring.control
            if (!c || c.enabled || !c.focus) {
                return
            }
            const next = c.nextItemInFocusChain(true)
            if (next && next !== c) {
                next.forceActiveFocus(Qt.TabFocusReason)
            }
        }
    }

    function scrollIntoView() {
        // The nearest ancestor that scrolls: a Flickable (or ListView, which is
        // one) exposes contentY alongside its contentItem.
        let flick = ring.parent
        while (flick && !(typeof flick.contentY === "number" && flick.contentItem)) {
            flick = flick.parent
        }
        if (!flick) {
            return
        }

        const top = ring.mapToItem(flick.contentItem, 0, 0).y
        const bottom = top + ring.height
        // The scrollable range of contentY, margins included.
        const first = flick.originY - flick.topMargin
        const last = flick.originY + flick.contentHeight - flick.height + flick.bottomMargin

        if (top < flick.contentY) {
            flick.contentY = Math.max(first, top)
        } else if (bottom > flick.contentY + flick.height) {
            flick.contentY = Math.max(first, Math.min(last, bottom - flick.height))
        }
    }
}
