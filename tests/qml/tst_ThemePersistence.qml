import QtQuick
import QtTest
import Countdown

// The theme and the Settings page's solver options survive a restart.
//
// A "restart" here is appRestart.afterRestart(expr): a second, throwaway
// engine wired like main.cpp's (fresh Preferences + LanguageManager reading
// the same QSettings store), evaluating `expr` against *its own* Theme /
// AppState singletons - not the live ones this test has been changing. See
// RestartHarness in tests/qml/tst_qml.cpp.
TestCase {
    id: testCase
    name: "ThemePersistence"

    Component { id: pageComponent; SettingsPage {} }

    function init() {
        Theme.mode = "system"
        AppState.minLen = 4
        AppState.maxResults = 60
        AppState.flagInexact = true
    }

    function cleanup() {
        init()
    }

    function test_defaultStaysSystem() {
        compare(appRestart.afterRestart("Theme.mode"), "system")
        compare(preferences.theme, "system")
    }

    function test_chosenThemeIsWrittenThroughAndSurvivesARestart() {
        Theme.mode = "dark"
        compare(preferences.theme, "dark")
        compare(appRestart.afterRestart("Theme.mode"), "dark")
        // Theme.dark is what every colour binds to: the restarted app is dark
        // from its very first evaluation, with no frame of "system" first.
        compare(appRestart.afterRestart("Theme.dark"), true)

        Theme.mode = "light"
        compare(appRestart.afterRestart("Theme.mode"), "light")
        compare(appRestart.afterRestart("Theme.dark"), false)

        Theme.mode = "system"
        compare(appRestart.afterRestart("Theme.mode"), "system")
    }

    function test_themeSegmentInSettingsPersists() {
        const page = createTemporaryObject(pageComponent, testCase)
        verify(page !== null)
        const themeSeg = findChild(page, "themeSeg")
        verify(themeSeg !== null)

        // Same signal a click on the "Dark" segment emits (see SegChip.qml).
        themeSeg.activated(1)
        compare(Theme.mode, "dark")
        compare(appRestart.afterRestart("Theme.mode"), "dark")

        themeSeg.activated(0)
        compare(appRestart.afterRestart("Theme.mode"), "light")
    }

    function test_unknownThemeIsNotStored() {
        Theme.mode = "dark"
        preferences.theme = "sepia"
        compare(preferences.theme, "dark")
        compare(appRestart.afterRestart("Theme.mode"), "dark")
    }

    function test_solverOptionsSurviveARestart() {
        compare(appRestart.afterRestart("AppState.minLen"), 4)
        compare(appRestart.afterRestart("AppState.maxResults"), 60)
        compare(appRestart.afterRestart("AppState.flagInexact"), true)

        AppState.minLen = 5
        AppState.maxResults = 120
        AppState.flagInexact = false
        compare(appRestart.afterRestart("AppState.minLen"), 5)
        compare(appRestart.afterRestart("AppState.maxResults"), 120)
        compare(appRestart.afterRestart("AppState.flagInexact"), false)
    }

    function test_settingsControlsPersistSolverOptions() {
        const page = createTemporaryObject(pageComponent, testCase, { width: 900, height: 700 })
        verify(page !== null)

        const minLenSeg = findChild(page, "minLenSeg")
        verify(minLenSeg !== null)
        minLenSeg.activated(2)   // "5+"
        compare(appRestart.afterRestart("AppState.minLen"), 5)

        // The switch is driven the way a keyboard user drives it: Space, then
        // Enter (which Switch alone would not act on).
        const flagSwitch = findChild(page, "flagSwitch")
        verify(flagSwitch !== null)
        flagSwitch.forceActiveFocus(Qt.TabFocusReason)
        verify(flagSwitch.activeFocus)

        keyClick(Qt.Key_Space)
        compare(AppState.flagInexact, false)
        compare(appRestart.afterRestart("AppState.flagInexact"), false)

        keyClick(Qt.Key_Return)
        compare(AppState.flagInexact, true)
        compare(appRestart.afterRestart("AppState.flagInexact"), true)
    }
}
