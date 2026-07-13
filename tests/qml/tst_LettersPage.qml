import QtQuick
import QtTest
import Countdown

TestCase {
    id: testCase
    name: "LettersPage"

    Component { id: pageComponent; LettersPage {} }

    function test_solveFindsAFallbackWord() {
        const page = createTemporaryObject(pageComponent, testCase)
        verify(page !== null)

        // "notice" is the only fallback word coverable by these six letters,
        // and is well above AppState's default minLen (4).
        const word = "notice"
        for (let i = 0; i < word.length; ++i) {
            page.addLetter(word.charAt(i).toUpperCase())
        }
        compare(page.letters.length, word.length)

        page.recalc()
        verify(page.result !== null)
        compare(page.result.total, 1)
        compare(page.result.longest, ["notice"])
    }

    function test_noMatches() {
        const page = createTemporaryObject(pageComponent, testCase)
        for (const ch of "XYZXYZ") {
            page.addLetter(ch)
        }
        page.recalc()

        verify(page.result !== null)
        compare(page.result.total, 0)
    }

    function test_backspaceRemovesLastLetter() {
        const page = createTemporaryObject(pageComponent, testCase)
        page.addLetter("A")
        page.addLetter("B")
        page.backspace()

        compare(page.letters.length, 1)
        compare(page.letters[0], "A")
    }

    function test_clearAllResetsState() {
        const page = createTemporaryObject(pageComponent, testCase)
        page.addLetter("A")
        page.clearAll()

        compare(page.letters.length, 0)
        compare(page.result, null)
    }

    function test_randomRackProducesAPlayableRound() {
        const page = createTemporaryObject(pageComponent, testCase)
        page.randomRack()

        compare(page.letters.length, 9)
        verify(page.result !== null)
        verify(page.result.total >= 1)
    }
}
