#include "solver_numbers_test.hpp"

#include <QStringList>
#include <QTest>
#include <QVariantList>
#include <QVariantMap>

void NumbersSolverTest::solveNumbers_exact() {
    const QVariantMap result = solver_.solveNumbers(QVariantList{50, 50}, 100);

    QCOMPARE(result["exact"].toBool(), true);
    QCOMPARE(result["value"].toLongLong(), 100LL);
    QCOMPARE(result["diff"].toLongLong(), 0LL);
    const QStringList steps = result["steps"].toStringList();
    QVERIFY(!steps.isEmpty());
    QCOMPARE(steps.first(), QStringLiteral("50 + 50 = 100"));
}

void NumbersSolverTest::solveNumbers_closestFallback() {
    // {1, 2} can reach at most 3 (1+2); nowhere near target 999, so this
    // exercises the "not exact" branch deterministically.
    const QVariantMap result = solver_.solveNumbers(QVariantList{1, 2}, 999);

    QCOMPARE(result["exact"].toBool(), false);
    QCOMPARE(result["value"].toLongLong(), 3LL);
    QCOMPARE(result["diff"].toLongLong(), 996LL);
}

void NumbersSolverTest::solveNumbers_emptyInput() {
    const QVariantMap result = solver_.solveNumbers(QVariantList{}, 500);

    QCOMPARE(result["exact"].toBool(), false);
    QCOMPARE(result["value"].toLongLong(), 0LL);
    QCOMPARE(result["diff"].toLongLong(), 500LL);
    QVERIFY(result["steps"].toStringList().isEmpty());
}
