import QtQuick
import QtTest
import Countdown

TestCase {
    id: testCase
    name: "Theme"

    function init() {
        Theme.mode = "system"
    }

    function cleanup() {
        Theme.mode = "system"
    }

    function test_defaultsToSystem() {
        compare(Theme.mode, "system")
    }

    function test_explicitLightAndDark() {
        Theme.mode = "light"
        compare(Theme.dark, false)

        Theme.mode = "dark"
        compare(Theme.dark, true)
    }

    function test_systemFollowsStyleHints() {
        Theme.mode = "system"
        // Compared against live Qt state (not a hardcoded expectation) so this
        // passes regardless of what the CI's offscreen QPA platform reports.
        compare(Theme.dark, Qt.styleHints.colorScheme === Qt.ColorScheme.Dark)
    }
}
