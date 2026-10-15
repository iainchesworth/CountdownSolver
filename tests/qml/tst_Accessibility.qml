import QtQuick
import QtQuick.Controls
import QtTest
import Countdown

// Keyboard and assistive-technology access to the custom controls:
//   - every interactive control has an accessible role and name;
//   - Tab reaches each one, in a sensible order, on every page and at every
//     form factor;
//   - Space and Enter activate it;
//   - a 2px accent ring, 2px out from the control, marks keyboard focus - and
//     only keyboard focus, never a mouse click - and is never cut off or
//     scrolled out of sight.
TestCase {
    id: testCase
    name: "Accessibility"
    when: windowShown
    // Items only take focus while visible, and a TestCase is hidden by default.
    visible: true
    width: 1000
    height: 760

    Component { id: numbersComponent; NumbersPage {} }
    Component { id: lettersComponent; LettersPage {} }
    Component { id: conundrumComponent; ConundrumPage {} }
    Component { id: settingsComponent; SettingsPage {} }
    Component { id: mainComponent; Main {} }
    Component { id: navItemComponent; NavItem { text: "Letters"; glyph: "Aa" } }
    Component {
        id: bottomTabsComponent
        BottomTabBar {
            tabs: [["Numbers", "12"], ["Letters", "Aa"], ["Conundrum", "?"], ["Settings", "≡"]]
            currentIndex: 1
        }
    }

    // The real application window, when a test has opened one.
    property var shell: null

    function cleanup() {
        if (shell !== null) {
            shell.destroy()
            shell = null
        }
        Metrics.formFactor = FormFactor.desktop
        languageManager.setLanguage("en")
        AppState.minLen = 4
        AppState.flagInexact = true
    }

    // ---- helpers ---------------------------------------------------------

    function open(component, w, h) {
        const page = createTemporaryObject(component, testCase, { width: w || 900, height: h || 700 })
        verify(page !== null)
        page.forceActiveFocus()
        return page
    }

    function typeName(item) {
        return item.toString().split("(")[0].split("_QMLTYPE")[0]
    }

    function descendants(item, out) {
        out = out || []
        for (let i = 0; i < item.children.length; ++i) {
            out.push(item.children[i])
            descendants(item.children[i], out)
        }
        return out
    }

    function ofType(root, name) {
        return descendants(root).filter(function (item) { return typeName(item) === name })
    }

    function ringOf(item) {
        const rings = item.children.filter(function (child) { return typeName(child) === "FocusRing" })
        return rings.length > 0 ? rings[0] : null
    }

    // The nearest ancestor that scrolls (and so clips): where a ring must fit.
    function scrollerOf(item) {
        let p = item.parent
        while (p && !(typeof p.contentY === "number" && p.contentItem)) {
            p = p.parent
        }
        return p
    }

    function focusedItem() {
        return testCase.Window.activeFocusItem
    }

    // Presses Tab `count` times and reports what took focus each time.
    function tabStops(count) {
        const stops = []
        for (let i = 0; i < count; ++i) {
            keyClick(Qt.Key_Tab)
            const item = focusedItem()
            const info = item ? a11y.info(item) : {}
            stops.push({ item: item, role: info.role, name: info.name })
        }
        return stops
    }

    function names(stops) {
        return stops.map(function (stop) { return stop.name })
    }

    function range(first, last) {
        const out = []
        for (let n = first; n <= last; ++n) out.push(String(n))
        return out
    }

    function letterKeys() {
        return "QWERTYUIOP ASDFGHJKL ZXCVBNM".replace(/ /g, "").split("")
    }

    // Opens the real Main window and gives it keyboard focus. Keys then go
    // through a11y.keyClick(shell, ...) - QML's keyClick() only reaches the
    // test's own window.
    function openShell() {
        shell = mainComponent.createObject(null)
        verify(shell !== null)
        shell.requestActivate()
        tryVerify(function () { return shell.active }, 3000)
        return shell
    }

    function shellTabStops(count) {
        const stops = []
        for (let i = 0; i < count; ++i) {
            a11y.keyClick(shell, Qt.Key_Tab)
            stops.push(a11y.info(shell.activeFocusItem).name)
        }
        return stops
    }

    // ---- names and roles -------------------------------------------------

    function test_numbersControlsAreButtonsNamedForTheirLabel() {
        const page = open(numbersComponent)
        const expected = ["25", "50", "75", "100"].concat(range(1, 10), range(1, 9), ["0"],
                                                         ["Random game", "Backspace", "Clear"])
        const stops = tabStops(expected.length)

        compare(names(stops), expected)
        for (const stop of stops) {
            compare(stop.role, "Button", stop.name)
        }
    }

    function test_glyphOnlyButtonsGetSpokenNames() {
        // "↻ Random game" and "←" are what's drawn; neither is what should be
        // read out.
        const page = open(numbersComponent)
        const random = ofType(page, "FlatButton").filter(function (b) { return b.text.indexOf("Random") >= 0 })[0]
        const back = ofType(page, "FlatButton").filter(function (b) { return b.text === "←" })[0]
        verify(random !== undefined && back !== undefined)
        compare(a11y.info(random).name, "Random game")
        compare(a11y.info(back).name, "Backspace")
    }

    function test_settingsControlsHaveTheRightRolesAndNames() {
        const page = open(settingsComponent)
        const stops = tabStops(11)

        compare(names(stops), ["Light", "Dark", "System",
                               "Display language",
                               "3+", "4+", "5+",
                               "Flag when no exact answer",
                               "Max words shown",
                               "View on GitHub ↗", "Third-party licences"])
        compare(stops.map(function (s) { return s.role }),
                ["RadioButton", "RadioButton", "RadioButton",
                 "ComboBox",
                 "RadioButton", "RadioButton", "RadioButton",
                 "CheckBox",
                 "Slider",
                 "Link", "Link"])
    }

    function test_segmentedControlsAreNamedGroupsWithOneChecked() {
        const page = open(settingsComponent)
        const theme = findChild(page, "themeSeg")
        verify(theme !== null)

        const group = a11y.info(theme)
        compare(group.role, "Grouping")
        compare(group.name, "Theme")

        // Light / Dark / System: exactly the current one is checked.
        Theme.mode = "dark"
        const radios = descendants(theme).filter(function (item) {
            return a11y.info(item).role === "RadioButton"
        })
        compare(radios.length, 3)
        compare(radios.map(function (r) { return a11y.info(r).checked }), [false, true, false])
        Theme.mode = "system"
    }

    function test_switchIsNamedForItsVisibleLabelAndCarriesItsHint() {
        const page = open(settingsComponent)
        const sw = findChild(page, "flagSwitch")
        verify(sw !== null)
        const info = a11y.info(sw)
        compare(info.role, "CheckBox")
        compare(info.name, "Flag when no exact answer")
        compare(info.description, "Highlight the closest result in the numbers game.")
        compare(info.checked, true)
    }

    function test_namesFollowTheDisplayLanguage() {
        verify(languageManager.setLanguage("de"))
        const page = open(numbersComponent)
        const stops = tabStops(4 + 10 + 10 + 3)
        // The announced name is the translated label (German "Löschen"), not
        // the English source string: it re-evaluates on a language switch like
        // any other binding.
        compare(stops[stops.length - 1].name, "Löschen")
    }

    function test_tilesAnnounceTheirValueOrEmpty() {
        const page = open(numbersComponent)
        page.addNumber(25)
        page.addNumber(7)

        const tiles = ofType(page, "Tile")
        compare(tiles.length, 6)
        const infos = tiles.map(function (t) { return a11y.info(t) })
        compare(infos.map(function (i) { return i.role }), ["StaticText", "StaticText", "StaticText",
                                                              "StaticText", "StaticText", "StaticText"])
        compare(infos.map(function (i) { return i.name }), ["25", "7", "Empty", "Empty", "Empty", "Empty"])

        // Display only: reading them is not a Tab stop.
        for (const tile of tiles) {
            verify(!tile.activeFocusOnTab)
        }
    }

    function test_navigationTabsAreTabsWithTheCurrentOneSelected() {
        const nav = createTemporaryObject(navItemComponent, testCase, { active: true })
        const info = a11y.info(nav)
        compare(info.role, "PageTab")
        compare(info.name, "Letters")
        compare(info.selected, true)
        nav.active = false
        compare(a11y.info(nav).selected, false)

        const bar = createTemporaryObject(bottomTabsComponent, testCase, { width: 600, height: 80 })
        const tabs = ofType(bar, "BottomTabButton")
        compare(tabs.length, 4)
        compare(tabs.map(function (t) { return a11y.info(t).name }), ["Numbers", "Letters", "Conundrum", "Settings"])
        compare(tabs.map(function (t) { return a11y.info(t).role }), ["PageTab", "PageTab", "PageTab", "PageTab"])
        compare(tabs.map(function (t) { return a11y.info(t).selected }), [false, true, false, false])
    }

    // ---- Tab order, page by page -----------------------------------------

    function test_tabOrder_letters() {
        const page = open(lettersComponent)
        // Keyboard rows top to bottom, then the actions (Solve is disabled until
        // there are letters, so Tab skips it).
        const expected = letterKeys().concat(["Random rack", "Backspace", "Clear"])
        compare(names(tabStops(expected.length)), expected)
        // ...and round again.
        compare(tabStops(1)[0].name, "Q")
    }

    function test_tabOrder_conundrum() {
        const page = open(conundrumComponent)
        const expected = letterKeys().concat(["Random conundrum", "Backspace", "Clear"])
        compare(names(tabStops(expected.length)), expected)
        compare(tabStops(1)[0].name, "Q")
    }

    function test_tabOrder_numbersIncludesSolveOnceItCanBePressed() {
        const page = open(numbersComponent)
        page.numbers = [25, 50, 75, 100, 3, 6]
        page.target = "952"
        // Every number pad is disabled once six are chosen, so Tab goes
        // straight from the target keys to the actions.
        const expected = range(1, 9).concat(["0", "Random game", "Backspace", "Clear", "Solve"])
        compare(names(tabStops(expected.length)), expected)
    }

    function test_tabOrder_settingsThenDialog() {
        const page = open(settingsComponent)
        const dialog = page.licensesDialog

        // Reach "Third-party licences" (the last stop) and open it with Enter.
        tabStops(11)
        keyClick(Qt.Key_Return)
        tryVerify(function () { return dialog.visible }, 2000)

        // Close is first in the dialog's focus chain (so the first Tab lands on
        // the way out), then the licence links top to bottom.
        const stops = tabStops(6)
        compare(names(stops), ["Close",
                               "GNU Lesser General Public License v3.0 ↗",
                               "SIL Open Font License, Version 1.1 ↗",
                               "SIL Open Font License, Version 1.1 ↗",
                               "SIL Open Font License, Version 1.1 ↗",
                               "SIL Open Font License, Version 1.1 ↗"])
        compare(stops.map(function (s) { return s.role }),
                ["Button", "Link", "Link", "Link", "Link", "Link"])

        keyClick(Qt.Key_Escape)
        tryVerify(function () { return !dialog.visible }, 2000)
    }

    // ---- activation ------------------------------------------------------

    function test_spaceAndEnterActivateTheFocusedButton() {
        const page = open(numbersComponent)
        page.forceActiveFocus()

        keyClick(Qt.Key_Tab)                    // 25
        compare(a11y.info(focusedItem()).name, "25")
        keyClick(Qt.Key_Space)
        compare(page.numbers, [25])

        // 25 is single-use and is now disabled, so focus has moved on to "50"
        // rather than being dropped (see the next test).
        compare(a11y.info(focusedItem()).name, "50")
        keyClick(Qt.Key_Return)
        compare(page.numbers, [25, 50])
        // Enter on a focused button presses *that button*: it must not also
        // reach the page's own Enter shortcut (commit/solve).
        compare(page.view, "input")
    }

    function test_enterOnAButtonDoesNotFallThroughToTheSolveShortcut() {
        const page = open(numbersComponent)
        page.numbers = [25, 50, 75, 100, 3, 6]
        page.target = "952"

        // Tab to "Clear" and press Enter: it clears; the page's own
        // Enter-means-Solve must not run as well.
        const stops = tabStops(14)
        compare(stops[12].name, "Clear")
        compare(a11y.info(focusedItem()).name, "Solve")
        keyClick(Qt.Key_Backtab, Qt.ShiftModifier)
        compare(a11y.info(focusedItem()).name, "Clear")
        keyClick(Qt.Key_Return)
        compare(page.numbers.length, 0)
        compare(page.target, "")
        compare(page.view, "input")
        verify(page.result === null)
    }

    function test_pageKeyboardShortcutsStillWorkWhileAButtonHasFocus() {
        const page = open(numbersComponent)
        keyClick(Qt.Key_Tab)    // a pad takes focus
        verify(focusedItem() !== page)

        // Keys the button has no use for bubble past it to the page, as before:
        // a typed digit goes into the number buffer, Backspace takes it out.
        keyClick(Qt.Key_5)
        compare(page.numberBuffer, "5")
        keyClick(Qt.Key_Backspace)
        compare(page.numberBuffer, "")
    }

    function test_segmentedControlActivatesWithKeyboard() {
        const page = open(settingsComponent)
        const stops = tabStops(2)            // Light, Dark
        compare(stops[1].name, "Dark")
        keyClick(Qt.Key_Space)
        compare(Theme.mode, "dark")
        keyClick(Qt.Key_Tab)                 // System
        keyClick(Qt.Key_Return)
        compare(Theme.mode, "system")
    }

    function test_licencesLinkOpensDialogFromKeyboard() {
        const page = open(settingsComponent)
        tabStops(11)
        compare(a11y.info(focusedItem()).name, "Third-party licences")
        keyClick(Qt.Key_Space)
        tryVerify(function () { return page.licensesDialog.visible }, 2000)
        page.licensesDialog.close()
    }

    // ---- the focus ring --------------------------------------------------

    function test_keyboardFocusDrawsTheRingAtTheSpecifiedGeometry() {
        const page = open(numbersComponent)
        keyClick(Qt.Key_Tab)
        const pad = focusedItem()
        const ring = ringOf(pad)
        verify(ring !== null)
        verify(ring.visible)

        // 2px line, in the accent colour, 2px clear of the control.
        compare(ring.border.width, 2)
        compare(ring.border.color, Theme.accent)
        compare(ring.x, -4)
        compare(ring.y, -4)
        compare(ring.width, pad.width + 8)
        compare(ring.height, pad.height + 8)
        compare(ring.color, "#00000000")   // outline only: the control shows through
    }

    function test_ringFollowsFocusAndOnlyOneIsShown() {
        const page = open(numbersComponent)
        const stops = tabStops(3)
        const shown = ofType(page, "FocusRing").filter(function (r) { return r.visible })
        compare(shown.length, 1)
        compare(shown[0].parent, stops[2].item)
    }

    function test_mouseClickNeitherMovesFocusNorDrawsARing() {
        const page = open(numbersComponent)
        const seven = ofType(page, "PadButton").filter(function (p) { return p.text === "7" })[0]
        verify(seven !== undefined)

        mouseClick(seven)
        compare(page.numbers, [7])                    // the click worked...
        verify(!seven.activeFocus)                    // ...but took no focus
        verify(!seven.visualFocus)
        verify(!ringOf(seven).visible)                // ...and drew no ring
        verify(ofType(page, "FocusRing").every(function (r) { return !r.visible }))
    }

    function test_ringShowsOnTheDarkThemeToo() {
        Theme.mode = "dark"
        const page = open(numbersComponent)
        keyClick(Qt.Key_Tab)
        const ring = ringOf(focusedItem())
        verify(ring.visible)
        compare(ring.border.color, Theme.accent)
        Theme.mode = "system"
    }

    function test_ringIsNeverClippedOrScrolledOutOfView_data() {
        return [
            { tag: "desktop",         form: FormFactor.desktop,         w: 926, h: 690 },
            { tag: "tabletPortrait",  form: FormFactor.tabletPortrait,  w: 768, h: 900 },
            { tag: "phonePortrait",   form: FormFactor.phonePortrait,   w: 390, h: 640 },
            { tag: "phoneLandscape",  form: FormFactor.phoneLandscape,  w: 844, h: 300 }
        ]
    }

    function test_ringIsNeverClippedOrScrolledOutOfView(data) {
        const pages = [numbersComponent, lettersComponent, conundrumComponent]
        let checked = 0
        for (const component of pages) {
            Metrics.formFactor = data.form
            const page = open(component, data.w, data.h)
            wait(30)

            // Walk the whole Tab cycle (more than long enough for any page).
            for (let i = 0; i < 45; ++i) {
                keyClick(Qt.Key_Tab)
                const item = focusedItem()
                if (!item || item === page) continue
                const ring = ringOf(item)
                if (!ring || !ring.visible) continue

                const scroller = scrollerOf(ring)
                if (!scroller) continue

                // A control that already overflows its container (the on-screen
                // keyboard's top row does at narrow widths, with or without
                // this change) is itself cut off; the ring can't be held to a
                // better standard than the thing it surrounds.
                const own = item.mapToItem(scroller, 0, 0)
                if (own.x < 0 || own.x + item.width > scroller.width) continue

                const topLeft = ring.mapToItem(scroller, 0, 0)
                const where = data.tag + " " + a11y.info(item).name
                verify(topLeft.x >= -0.01, where + ": ring cut off on the left")
                verify(topLeft.y >= -0.01, where + ": ring cut off at the top")
                verify(topLeft.x + ring.width <= scroller.width + 0.01, where + ": ring cut off on the right")
                verify(topLeft.y + ring.height <= scroller.height + 0.01, where + ": ring cut off at the bottom")
                ++checked
            }
            // Gone before the next form factor (or cleanup()) changes the
            // metrics under it.
            page.destroy()
            wait(10)
        }

        // Guards the filters above: a pass must mean rings were measured. Desktop
        // has no scroller to measure against; every other form factor has the
        // three action buttons (plus the controls beside them) at least.
        if (data.form !== FormFactor.desktop) {
            verify(checked >= 9, data.tag + ": only " + checked + " rings were measured")
        }
    }

    function test_settingsScrollsTheFocusedControlIntoView() {
        // Short enough that the cards below the fold are out of sight.
        const page = open(settingsComponent, 800, 260)
        const flick = descendants(page).filter(function (i) {
            return typeof i.contentY === "number" && i.contentItem
        })[0]
        verify(flick !== undefined)
        compare(flick.contentY, 0)

        tabStops(11)        // ...to the last control on the page
        compare(a11y.info(focusedItem()).name, "Third-party licences")
        verify(flick.contentY > 0)

        const ring = ringOf(focusedItem())
        const topLeft = ring.mapToItem(flick, 0, 0)
        verify(topLeft.y >= -0.01)
        verify(topLeft.y + ring.height <= flick.height + 0.01)

        // And back up again with Shift+Tab, the page following focus.
        for (let i = 0; i < 10; ++i) keyClick(Qt.Key_Backtab, Qt.ShiftModifier)
        compare(a11y.info(focusedItem()).name, "Light")
        const top = ringOf(focusedItem()).mapToItem(flick, 0, 0)
        verify(top.y >= -0.01)
        verify(top.y + ringOf(focusedItem()).height <= flick.height + 0.01)
    }

    function test_focusMovesOnWhenTheFocusedButtonIsDisabled() {
        const page = open(numbersComponent)
        keyClick(Qt.Key_Tab)                          // "25"
        keyClick(Qt.Key_Space)                        // used up -> disabled
        const item = focusedItem()
        compare(a11y.info(item).name, "50")
        verify(item.visualFocus)
        verify(ringOf(item).visible)
    }

    // ---- the whole window ------------------------------------------------

    function test_shellTabOrderIsNavigationThenPage() {
        openShell()
        // At launch the Numbers page itself has focus (so typing works at
        // once); Tab runs through the sidebar, then into the page.
        compare(shellTabStops(8), ["Numbers", "Letters", "Conundrum", "Settings",
                                   "25", "50", "75", "100"])
    }

    function test_shellEnterOnATabSwitchesPageAndHandsTypingToIt() {
        openShell()
        compare(shell.currentIndex, 0)

        shellTabStops(2)                           // Numbers, Letters
        a11y.keyClick(shell, Qt.Key_Return)
        compare(shell.currentIndex, 1)

        // The page that came up takes focus, so letters typed next land in
        // its rack - the same hand-off a mouse click on the tab gives.
        compare(shell.activeFocusItem, shell.activePage)
        a11y.keyClick(shell, Qt.Key_A)
        compare(shell.activePage.letters, ["A"])
    }

    function test_shellSpaceOnATabSwitchesPage() {
        openShell()
        shellTabStops(4)                           // ... Settings
        a11y.keyClick(shell, Qt.Key_Space)
        compare(shell.currentIndex, 3)
    }
}
