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
        // with (10-25s observed vs a few hundred ms unsanitized), so a short
        // fixed tryVerify() timeout here previously made this test flake.
        //
        // verify(page.busy) confirms recalc() actually kicked off a solve
        // synchronously (recalc() sets busy before returning). The generous
        // tryVerify() below then drains that solve before this function
        // returns and `page` is destroyed - it must run to completion here,
        // not just start, or the still-running background solve finishes
        // later and delivers its result through the shared `solver`
        // singleton's Connections to whichever *other* test's page happens
        // to be alive and listening at that moment. That isn't hypothetical:
        // dropping this wait once let this round's stale result land on
        // test_solveExactMatch's page instead (QtQuickTest doesn't run
        // test functions in source order), failing it with an unrelated
        // value. The timeout is generous rather than tight on purpose - it
        // only needs to bound worst-case runtime, not catch a real failure,
        // since solveNumbers() always resolves for a board-legal round.
        verify(page.busy)
        tryVerify(function () { return !page.busy }, 60000)
    }
}
