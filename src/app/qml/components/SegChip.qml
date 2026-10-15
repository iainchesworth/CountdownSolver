import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Single segment of SegControl. A standalone file (not an inline `component`
// block inside SegControl.qml), and deliberately NOT using a `required
// property` fed from the Repeater's `index`/`modelData` at instantiation -
// see git history/PR discussion for the two bugs this works around:
// (1) an inline `component` used as a Repeater delegate with a `required
// property` rendered every instance with the same value; (2) even as a real
// file, a `required property` used to *index into an array* (`options[idx]`)
// silently returned the same (first-instance) element for every delegate,
// while a plain scalar comparison using the same required property (e.g.
// `idx === currentIndex`) worked correctly - something in this project's AOT
// QML compilation specifically miscompiles the array-subscript case. Fix:
// don't declare `chipIndex`/the label as `required property` at all - read
// the Repeater's *implicit* `index`/`modelData` context properties directly,
// the same proven-safe pattern Tile/PadButton delegates already use
// throughout this codebase (SegControl passes `model: seg.options` - the
// array itself - so `modelData` here already *is* the label, no indexing).
Rectangle {
    id: chip
    property var owner
    readonly property bool active: index === owner.currentIndex
    Layout.fillWidth: owner.stretch
    Layout.fillHeight: owner.stretch
    // A ternary can't "leave a property unset" (assigning `undefined` to a
    // numeric property logs "Unable to assign [undefined] to double") - an
    // inactive Binding never assigns at all, same idiom Main.qml already
    // uses for its desktop-only min/max window size Bindings.
    Binding on implicitWidth {
        value: label.implicitWidth + owner.hPadding * 2
        when: !owner.stretch
    }
    implicitHeight: owner.segHeight
    radius: owner.segRadius
    color: chip.active ? Theme.accent : "transparent"
    Text {
        id: label
        anchors.centerIn: parent
        text: modelData
        font.family: Theme.sans
        font.pixelSize: chip.owner.pixelSize
        font.weight: Font.DemiBold
        color: chip.active ? Theme.accentInk : Theme.muted
    }
    // The segment's interactive surface: a flat, chrome-less Button laid over
    // the whole chip (the chip itself stays a plain Rectangle, for the reasons
    // in the header comment). It is what makes a segment reachable with Tab,
    // activatable with Space/Enter, announced as a radio button, and able to
    // show a focus ring - none of which the MouseArea it replaced could do.
    Button {
        id: hit
        anchors.fill: parent
        flat: true
        padding: 0
        background: Item {}
        contentItem: Item {}

        // Tab reaches it; a click doesn't take keyboard focus (so the page
        // keeps receiving typed keys, as before) and never draws the ring.
        focusPolicy: Qt.TabFocus

        // The segments are one choice among several: radio buttons, with the
        // current one checked, named by their visible label.
        Accessible.role: Accessible.RadioButton
        Accessible.name: label.text
        Accessible.checked: chip.active

        HoverHandler { cursorShape: Qt.PointingHandCursor }

        // Only emit activated() - never assign owner.currentIndex directly.
        // currentIndex is always caller-bound to external state (Theme.mode,
        // AppState.*, a page's view); an imperative write here would
        // permanently sever that binding (QML replaces a property's binding
        // with a static value on any imperative assignment), so currentIndex
        // would stop tracking external state changed by anything other than
        // this control from then on.
        onClicked: chip.owner.activated(index)
        Keys.onReturnPressed: hit.clicked()
        Keys.onEnterPressed: hit.clicked()

        FocusRing { controlRadius: chip.owner.segRadius }
    }
}
