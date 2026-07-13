import QtQuick
import QtTest
import Countdown

TestCase {
    id: testCase
    name: "ConundrumPage"

    Component { id: pageComponent; ConundrumPage {} }

    function test_solveFindsTheAnagram() {
        const page = createTemporaryObject(pageComponent, testCase)
        verify(page !== null)

        // "countdown"'s own letters are a unique full anagram among the
        // fallback words, so this always resolves to exactly that word.
        const word = "countdown"
        for (let i = 0; i < word.length; ++i) {
            page.addLetter(word.charAt(i).toUpperCase())
        }
        compare(page.letters.length, 9)

        page.recalc()
        verify(page.result !== null)
        compare(page.result.found, true)
        compare(page.result.answers, ["countdown"])
    }

    function test_notFound() {
        const page = createTemporaryObject(pageComponent, testCase)
        for (const ch of "XYZXYZXYZ") {
            page.addLetter(ch)
        }
        page.recalc()

        verify(page.result !== null)
        compare(page.result.found, false)
        compare(page.result.answers.length, 0)
    }

    function test_clearAllResetsState() {
        const page = createTemporaryObject(pageComponent, testCase)
        page.addLetter("A")
        page.clearAll()

        compare(page.letters.length, 0)
        compare(page.result, null)
    }

    function test_randomConundrumProducesAPlayableRound() {
        const page = createTemporaryObject(pageComponent, testCase)
        page.randomConundrum()

        compare(page.letters.length, 9)
        verify(page.result !== null)
        // A shuffled fallback word is always a full anagram of itself.
        compare(page.result.found, true)
    }
}
