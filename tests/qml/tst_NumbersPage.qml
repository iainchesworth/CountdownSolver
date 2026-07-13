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

        page.addNumber(50)
        page.addNumber(50)
        page.addNumber(1)
        page.addNumber(1)
        page.addNumber(1)
        page.addNumber(1)
        compare(page.numbers.length, 6)

        page.targetDigit("1")
        page.targetDigit("0")
        page.targetDigit("0")
        compare(page.target, "100")

        page.recalc()
        verify(page.result !== null)
        compare(page.result.exact, true)
        compare(page.result.value, 100)
        compare(page.result.diff, 0)
        verify(page.result.steps.length > 0)
    }

    function test_clearAllResetsState() {
        const page = createTemporaryObject(pageComponent, testCase)
        page.addNumber(50)
        page.addNumber(50)
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
        verify(/^[0-9]{1,3}$/.test(page.target))
        verify(page.result !== null)
    }
}
