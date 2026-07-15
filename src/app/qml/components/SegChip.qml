import QtQuick
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
    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        // Only emit activated() - never assign owner.currentIndex directly.
        // currentIndex is always caller-bound to external state (Theme.mode,
        // AppState.*, a page's view); an imperative write here would
        // permanently sever that binding (QML replaces a property's binding
        // with a static value on any imperative assignment), so currentIndex
        // would stop tracking external state changed by anything other than
        // this control from then on.
        onClicked: chip.owner.activated(index)
    }
}
