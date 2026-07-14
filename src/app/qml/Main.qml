import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Application shell: sidebar navigation + page stack. The OS provides the
// window frame (no faux titlebar), matching a native desktop look.
ApplicationWindow {
    id: win
    visible: true
    width: 1140
    height: 740
    minimumWidth: 1140
    minimumHeight: 740
    maximumWidth: 1140
    maximumHeight: 740
    title: qsTr("Countdown Solver")
    color: Theme.bg

    // Mirrors every anchor/Row/ColumnLayout in the window (and, via
    // childrenInherit, every descendant item) for right-to-left languages
    // (Arabic, Hebrew, Yiddish). Qt.application.layoutDirection follows the
    // application's layout direction, which LanguageManager sets from the
    // active language on every switch (see language_manager.cpp).
    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    property int currentIndex: 0
    readonly property var pages: [
        { title: qsTr("Numbers"),   sub: qsTr("Reach the target using each number at most once.") },
        { title: qsTr("Letters"),   sub: qsTr("Find every word hiding in your nine letters.") },
        { title: qsTr("Conundrum"), sub: qsTr("Unscramble the nine letters into one word.") },
        { title: qsTr("Settings"),  sub: qsTr("Tune the solver and how results are shown.") }
    ]

    RowLayout {
        anchors.fill: parent
        spacing: 0

        // ---------- sidebar ----------
        Rectangle {
            Layout.preferredWidth: 214
            Layout.fillHeight: true
            color: Theme.sidebar

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 12
                spacing: 3

                RowLayout {
                    Layout.leftMargin: 6
                    Layout.topMargin: 6
                    Layout.bottomMargin: 12
                    spacing: 11
                    Rectangle {
                        width: 34; height: 34; radius: 9
                        color: Theme.accent
                        Text {
                            anchors.centerIn: parent; text: "C"
                            color: Theme.accentInk; font.family: Theme.mono
                            font.pixelSize: 15; font.weight: Font.Bold
                        }
                    }
                    ColumnLayout {
                        spacing: 2
                        Text { text: qsTr("Countdown"); color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.Bold }
                        Text { text: qsTr("SOLVER"); color: Theme.faint; font.family: Theme.mono; font.pixelSize: 11; font.letterSpacing: 1 }
                    }
                }

                Repeater {
                    model: [ [qsTr("Numbers"),"12"], [qsTr("Letters"),"Aa"], [qsTr("Conundrum"),"?"], [qsTr("Settings"),"\u2261"] ]
                    delegate: NavItem {
                        Layout.fillWidth: true
                        text: modelData[0]
                        glyph: modelData[1]
                        active: win.currentIndex === index
                        onClicked: win.currentIndex = index
                    }
                }

                Item { Layout.fillHeight: true }
                Rectangle { Layout.fillWidth: true; height: 1; color: Theme.border }
                Text {
                    Layout.topMargin: 8; Layout.leftMargin: 8
                    // Reading solver.dictionariesReady (a real NOTIFYing
                    // property) in this binding is what makes it re-evaluate
                    // once the deferred load finishes - dictionaryWordCount()
                    // alone is a plain invokable call QML can't track.
                    text: solver.dictionariesReady
                          ? qsTr("%1 words loaded").arg(Number(solver.dictionaryWordCount()).toLocaleString(Qt.locale(), "f", 0))
                          : qsTr("Loading…")
                    color: Theme.faint; font.family: Theme.mono; font.pixelSize: 11
                }
                RowLayout {
                    Layout.leftMargin: 8
                    Layout.topMargin: 2
                    spacing: 6
                    Text {
                        text: "v" + solver.shortVersion()
                        color: Theme.faint; font.family: Theme.mono; font.pixelSize: 11
                    }
                    Rectangle {
                        visible: solver.isDirty()
                        radius: 7
                        implicitHeight: 15; implicitWidth: dirtyLabel.implicitWidth + 12
                        color: Theme.warnBg
                        Text {
                            id: dirtyLabel
                            anchors.centerIn: parent
                            text: qsTr("dirty")
                            color: Theme.warnInk; font.family: Theme.mono
                            font.pixelSize: 9; font.weight: Font.DemiBold
                        }
                    }
                }
            }
        }

        // ---------- main ----------
        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            ColumnLayout {
                Layout.fillWidth: true
                Layout.margins: 24
                Layout.bottomMargin: 8
                spacing: 4
                Text { text: win.pages[win.currentIndex].title; color: Theme.ink; font.family: Theme.sans; font.pixelSize: 22; font.weight: Font.Bold }
                Text { text: win.pages[win.currentIndex].sub;   color: Theme.muted; font.family: Theme.sans; font.pixelSize: 14 }
            }

            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: win.currentIndex
                NumbersPage {}
                LettersPage {}
                ConundrumPage {}
                SettingsPage {}
            }
        }
    }
}
