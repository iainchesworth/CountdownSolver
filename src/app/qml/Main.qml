import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Window

// Application shell: sidebar navigation + page stack. The OS provides the
// window frame (no faux titlebar), matching a native desktop look.
ApplicationWindow {
    id: win
    visible: true

    // Mobile builds (tablet, landscape-only - see AndroidManifest.xml/Info.plist.in)
    // get a fullscreen window sized to the device instead of the fixed desktop
    // size below; the layout itself is unchanged; RowLayout/ColumnLayout fill
    // bindings throughout already adapt to whatever size the window ends up.
    readonly property bool isMobile: Qt.platform.os === "android" || Qt.platform.os === "ios"

    // Drives the responsive shell (sidebar+two-pane vs bottom-tabs+stacked)
    // from actual window size/orientation rather than Qt.platform.os, so a
    // resized/rotated window re-flows live instead of being locked to
    // whatever chrome the OS started with.
    readonly property int formFactor:
          !isMobile                          ? FormFactor.desktop
        : width >= 900 && width > height     ? FormFactor.tabletLandscape
        : width >= 600 && height >= width    ? FormFactor.tabletPortrait
        : height > width                     ? FormFactor.phonePortrait
        :                                       FormFactor.phoneLandscape
    Binding { target: Metrics; property: "formFactor"; value: win.formFactor }

    // On mobile, width/height are intentionally left unset rather than bound
    // to Screen.width/Screen.height: with visibility already FullScreen, the
    // platform owns the window's actual geometry, and Screen.width/height
    // don't reliably track live rotation on iOS (observed on a real device:
    // width/height stuck at a stale pre-rotation size, leaving the surface
    // letterboxed - black bars on two edges, content clipped on the other
    // two, the exact bars/edges swapping with orientation). Forcing width/
    // height here just fights FullScreen instead of matching it.
    Binding { target: win; property: "width";  value: 1140; when: !win.isMobile }
    Binding { target: win; property: "height"; value: 740;  when: !win.isMobile }
    visibility: isMobile ? ApplicationWindow.FullScreen : ApplicationWindow.Windowed

    // Bindings that only apply when !isMobile - a ternary can't "leave a
    // property unset", but an inactive Binding never assigns at all, leaving
    // these at Qt's own default (0 / effectively unbounded) on mobile.
    Binding { target: win; property: "minimumWidth";  value: 1140; when: !win.isMobile }
    Binding { target: win; property: "minimumHeight"; value: 740;  when: !win.isMobile }
    Binding { target: win; property: "maximumWidth";  value: 1140; when: !win.isMobile }
    Binding { target: win; property: "maximumHeight"; value: 740;  when: !win.isMobile }

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
    // Shared by the sidebar's NavItem repeater and the mobile BottomTabBar,
    // so the two navigation UIs can never drift out of sync.
    readonly property var navTabs: [
        [qsTr("Numbers"), "12"], [qsTr("Letters"), "Aa"], [qsTr("Conundrum"), "?"], [qsTr("Settings"), "≡"]
    ]

    readonly property bool showBottomTabs: formFactor !== FormFactor.desktop && formFactor !== FormFactor.tabletLandscape
    // Games only - Settings (index 3) has no Input/Results split.
    readonly property bool showViewToggle: showBottomTabs && currentIndex !== 3
    readonly property var activePage: [numbersPageItem, lettersPageItem, conundrumPageItem, null][currentIndex]

    function activateTab(index) {
        currentIndex = index
        const target = [numbersPageItem, lettersPageItem, conundrumPageItem, null][index]
        if (target) target.view = "input"
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            // ---------- sidebar ----------
            Rectangle {
                visible: formFactor === FormFactor.desktop || formFactor === FormFactor.tabletLandscape
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
                        model: win.navTabs
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

                // Compact combined title+toggle bar (phone-landscape only) -
                // replaces the header+toggle blocks below, since there's no room
                // for both as separate rows in ~390px of window height.
                PageHeader {
                    Layout.fillWidth: true
                    visible: formFactor === FormFactor.phoneLandscape
                    title: win.pages[win.currentIndex].title
                    subtitle: win.pages[win.currentIndex].sub
                    showToggle: win.showViewToggle
                    currentView: win.activePage ? win.activePage.view : "input"
                    hasResult: win.activePage ? win.activePage.hasResult : false
                    onViewActivated: view => { if (win.activePage) win.activePage.view = view }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    visible: formFactor !== FormFactor.phoneLandscape
                    Layout.topMargin: Metrics.compactIndex < 0 ? 24 : Metrics.headerPadding.top[Metrics.compactIndex]
                    Layout.leftMargin: Metrics.compactIndex < 0 ? 24 : Metrics.headerPadding.side[Metrics.compactIndex]
                    Layout.rightMargin: Metrics.compactIndex < 0 ? 24 : Metrics.headerPadding.side[Metrics.compactIndex]
                    Layout.bottomMargin: Metrics.compactIndex < 0 ? 8 : Metrics.headerPadding.bottom[Metrics.compactIndex]
                    spacing: 4
                    Text {
                        text: win.pages[win.currentIndex].title; color: Theme.ink; font.family: Theme.sans
                        font.pixelSize: Metrics.compactIndex < 0 ? 22 : Metrics.headerTitle.font[Metrics.compactIndex]
                        font.weight: Font.Bold
                    }
                    Text {
                        text: win.pages[win.currentIndex].sub; color: Theme.muted; font.family: Theme.sans
                        font.pixelSize: Metrics.compactIndex < 0 ? 14 : Metrics.headerSubtitle.font[Metrics.compactIndex]
                    }
                }

                // Own bar under the header (tablet-portrait/phone-portrait only -
                // phone-landscape's toggle lives inline in PageHeader above, and
                // desktop/tablet-landscape show input/results side by side).
                ViewToggle {
                    Layout.fillWidth: true
                    Layout.leftMargin: Metrics.compactIndex < 0 ? 0 : Metrics.headerPadding.side[Metrics.compactIndex]
                    Layout.rightMargin: Metrics.compactIndex < 0 ? 0 : Metrics.headerPadding.side[Metrics.compactIndex]
                    Layout.bottomMargin: 12
                    visible: win.showViewToggle && formFactor !== FormFactor.phoneLandscape
                    currentView: win.activePage ? win.activePage.view : "input"
                    hasResult: win.activePage ? win.activePage.hasResult : false
                    onViewActivated: view => { if (win.activePage) win.activePage.view = view }
                }

                StackLayout {
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    currentIndex: win.currentIndex
                    NumbersPage   { id: numbersPageItem }
                    LettersPage   { id: lettersPageItem }
                    ConundrumPage { id: conundrumPageItem }
                    SettingsPage  {}
                }
            }
        }

        BottomTabBar {
            Layout.fillWidth: true
            visible: win.showBottomTabs
            tabs: win.navTabs
            currentIndex: win.currentIndex
            onTabActivated: index => win.activateTab(index)
        }
    }
}
