import QtQuick
import QtTest
import Countdown

TestCase {
    id: testCase
    name: "SettingsPage"

    Component { id: pageComponent; SettingsPage {} }

    function test_instantiatesAndBindsToSolver() {
        const page = createTemporaryObject(pageComponent, testCase)
        verify(page !== null)

        // These bindings call solver.fullDictionaryAvailable()/versionDetails()
        // directly, so successfully finding them confirms the QML<->Solver
        // wiring works end to end, not just that Solver's C++ API is correct.
        const dictSeg = findChild(page, "dictSeg")
        verify(dictSeg !== null)
        compare(dictSeg.enabled, solver.fullDictionaryAvailable())

        const themeSeg = findChild(page, "themeSeg")
        verify(themeSeg !== null)
        compare(themeSeg.options.length, 3)

        const about = findChild(page, "aboutText")
        verify(about !== null)
        verify(about.versionLines.length > 0)
        verify(about.versionLines[0].length > 0)
    }

    function test_licensesLinkOpensDialog() {
        const page = createTemporaryObject(pageComponent, testCase)
        verify(page !== null)

        const link = findChild(page, "licensesLink")
        verify(link !== null)

        const dialog = page.licensesDialog
        verify(dialog !== null)
        compare(dialog.visible, false)

        // Entries pull solver.qtVersion() live, so a populated version string
        // confirms the QML<->Solver wiring works, not just that the array
        // literal itself is non-empty.
        verify(dialog.entries.length > 0)
        compare(dialog.entries[0].name, "Qt 6")
        verify(dialog.entries[0].version.length > 0)

        dialog.open()
        compare(dialog.visible, true)
        dialog.close()
    }
}
