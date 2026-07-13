import QtQuick
import QtTest
import Countdown

TestCase {
    id: testCase
    name: "NumbersPage"

    Component { id: pageComponent; NumbersPage {} }

    function test_initialState() {
        const page = createTemporaryObject(pageComponent, testCase)
        verify(page !== null)
        compare(page.numbers.length, 0)
        compare(page.target, "")
        compare(page.result, null)
    }

    function test_solveExactMatch() {
        const page = createTemporaryObject(pageComponent, testCase)
        verify(page !== null)

        // The real board supplies only one tile each of 25/50/75/100, so
        // 25 + 75 (each used once) is the exact-solve combination here,
        // padded with four small numbers (each also used at most once) to
        // reach the required six.
        page.addNumber(25)
        page.addNumber(75)
        page.addNumber(1)
        page.addNumber(2)
        page.addNumber(3)
        page.addNumber(4)
        compare(page.numbers.length, 6)

        page.targetDigit("1")
        page.targetDigit("0")
        page.targetDigit("0")
        compare(page.target, "100")

        page.recalc()
        // Solving now runs on a worker thread (Solver::solveNumbersAsync);
        // the result arrives via a signal once the event loop processes it.
        tryVerify(function () { return page.result !== null }, 5000)
        compare(page.result.exact, true)
        compare(page.result.value, 100)
        compare(page.result.diff, 0)
        verify(page.result.steps.length > 0)
    }

    function test_clearAllResetsState() {
        const page = createTemporaryObject(pageComponent, testCase)
        page.addNumber(50)
        page.addNumber(75)
        page.targetDigit("5")
        page.clearAll()

        compare(page.numbers.length, 0)
        compare(page.target, "")
        compare(page.result, null)
    }

    function test_randomGameProducesAPlayableRound() {
        const page = createTemporaryObject(pageComponent, testCase)
        page.randomGame()

        compare(page.numbers.length, 6)
        verify(page.targetIsValid())
        // randomGame() draws a genuinely random round, which - unlike the
        // fixed round below - may have no exact solution. search() (see
        // numbers_game.cpp) only stops early on an exact match; lacking one,
        // it exhaustively walks the whole combination tree. Measured against
        // this repo's actual solveNumbers(), that non-exact path is ~50x
        // slower under the ASan+UBSan instrumentation every CI job builds
        // with (10-25s observed vs a few hundred ms unsanitized) - no fixed
        // tryVerify() timeout is both CI-safe and tight, so this only checks
        // that randomGame() -> recalc() actually kicked off a solve
        // (synchronous - recalc() sets busy before returning). The async
        // wiring through to a delivered result is covered deterministically
        // by test_solveExactMatch's fixed, fast-to-solve round.
        verify(page.busy)
    }
}
