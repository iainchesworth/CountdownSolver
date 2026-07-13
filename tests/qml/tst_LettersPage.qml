import QtQuick
import QtTest
import Countdown

TestCase {
    id: testCase
    name: "LettersPage"

    Component { id: pageComponent; LettersPage {} }

    // Every letter in `word` occurs in `rack` at least as often - i.e. `word`
    // is genuinely spellable from `rack`'s letters. Used to check results
    // against the real (large) bundled dictionary without depending on its
    // exact contents.
    function canSpell(rack, word) {
        const counts = {}
        for (const ch of rack.toLowerCase()) counts[ch] = (counts[ch] || 0) + 1
        for (const ch of word.toLowerCase()) {
            if (!counts[ch]) return false
            counts[ch] -= 1
        }
        return true
    }

    function test_solveFindsRichRackWords() {
        const page = createTemporaryObject(pageComponent, testCase)
        verify(page !== null)

        // "creations" is a letter-rich 9-letter rack (the LettersPage cap);
        // exact word content isn't asserted (the dictionary can change
        // independently of this test) - every structural invariant of the
        // response is instead.
        const word = "creations"
        for (let i = 0; i < word.length; ++i) {
            page.addLetter(word.charAt(i).toUpperCase())
        }
        compare(page.letters.length, word.length)

        page.recalc()
        verify(page.result !== null)
        verify(page.result.total > 0)
        verify(page.result.longest.length > 0)
        for (const w of page.result.longest) {
            compare(w.length, page.result.maxLen)
            verify(canSpell(word, w))
        }
        for (const group of page.result.groups) {
            for (const w of group.words) {
                compare(w.length, group.len)
                verify(canSpell(word, w))
            }
        }
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

        // randomRack() now draws from a weighted letter-tile pool
        // independent of the dictionary (matching the real show), so a
        // given draw isn't guaranteed to spell any word at all - only that
        // recalc() ran and produced a well-formed (if possibly empty) result.
        compare(page.letters.length, 9)
        verify(page.result !== null)
        verify(page.result.total >= 0)
        for (const w of page.result.longest) {
            verify(canSpell(page.letters.join(""), w))
        }
    }
}
