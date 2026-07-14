import QtQuick
import QtQuick.Layouts
import QtTest
import Countdown

// Regression coverage for a bug found on a packaged Debian build: rapidly
// switching languages in Settings (including LTR<->RTL transitions) while
// flipping tabs could log QML TypeErrors ("Cannot read property
// 'currentLanguage' of null" / "Cannot call method 'availableLanguages' of
// null") from Theme.qml, LetterRackInput.qml and SettingsPage.qml - the
// `languageManager` context property momentarily read as null during/around
// a language switch. Root cause wasn't conclusively pinned down (extensive
// repro attempts here never reproduced the exact race), but a real ordering
// bug was found and fixed in LanguageManager::setLanguage() (retranslate()
// ran before current_language_ was updated), and every direct
// languageManager.* read in QML was given a defensive null-guard. This test
// exercises the same shape of interaction (real ComboBox popup open/select/
// close cycles, tab flips, LTR/RTL transitions, zero settle delay) that
// reproduced the report, and fails on any QML warning matching the crash
// signatures above - a permanent tripwire in case the underlying race is
// ever hit again in CI, on top of the null-guards' direct protection.
TestCase {
    id: testCase
    name: "LanguageSwitching"
    when: windowShown
    width: 1140
    height: 740

    Component {
        id: shellComponent
        Item {
            id: win
            anchors.fill: parent
            // Matches Main.qml's LayoutMirroring wiring - RTL languages
            // (ar/he/yi) are part of what's being exercised here.
            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            property int currentIndex: 0

            StackLayout {
                anchors.fill: parent
                currentIndex: win.currentIndex
                NumbersPage {}
                LettersPage {}
                ConundrumPage {}
                SettingsPage {}
            }
        }
    }

    property var shell: null

    function selectLanguageViaCombo(code) {
        const combo = findChild(shell, "langCombo")
        verify(combo !== null)

        const languages = languageManager.availableLanguages()
        let idx = -1
        for (let i = 0; i < languages.length; ++i) {
            if (languages[i].code === code) { idx = i; break }
        }
        verify(idx >= 0)

        // Opens the popup (creates the ListView's delegate items via
        // combo.delegateModel), selects the item exactly like
        // ComboBoxPrivate does on a real click (set currentIndex then emit
        // activated - onActivated: languageManager.setLanguage(currentValue)
        // is the same code a real click runs), then closes (which nulls the
        // popup ListView's model per SettingsPage.qml, destroying delegates).
        combo.popup.open()
        tryVerify(function () { return combo.popup.visible }, 2000)
        combo.currentIndex = idx
        combo.activated(idx)
        combo.popup.close()
        tryVerify(function () { return !combo.popup.visible }, 2000)
    }

    function test_rapidLanguageAndTabSwitchingLogsNoWarnings() {
        failOnWarning(/TypeError|Cannot read property|Cannot call method/)

        shell = shellComponent.createObject(testCase)
        verify(shell !== null)
        wait(50)

        // Show Settings first (tab index 3) so the combo is reachable.
        shell.currentIndex = 3
        wait(20)

        // LTR -> RTL -> LTR -> RTL transitions, mixed with plain LTR
        // switches, matching the report's en -> yi -> ar -> en shape.
        const sequence = ["yi", "ar", "en", "he", "ar", "yi", "en", "de", "ar", "en"]
        let tab = 3
        for (const code of sequence) {
            selectLanguageViaCombo(code)
            compare(languageManager.currentLanguage, code)

            // Flip to another tab and back with no settle delay, like
            // impatient rapid clicking between language changes.
            tab = (tab + 1) % 4
            shell.currentIndex = tab
            shell.currentIndex = 3
        }
    }
}
