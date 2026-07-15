import QtQuick

// A single accent-filled word chip (LettersPage's "longest word(s)" list).
// Used directly as a Repeater delegate (`model: <array of words>`) reading
// the implicit `modelData` context property - deliberately NOT a `required
// property` assigned at instantiation (`AccentChip { word: modelData }`),
// per the finding in SegChip.qml: a required property fed a value at
// delegate instantiation was found unreliable under this project's AOT QML
// compilation, while the Repeater's own implicit index/modelData context
// properties work correctly everywhere else in this codebase.
Rectangle {
    id: root
    radius: 9; color: Theme.accent; height: 44
    width: label.implicitWidth + 36
    Text {
        id: label
        anchors.centerIn: parent
        text: modelData
        color: Theme.accentInk; font.family: Theme.sans; font.pixelSize: 20
        font.weight: Font.DemiBold; font.capitalization: Font.AllUppercase; font.letterSpacing: 1.5
    }
}
