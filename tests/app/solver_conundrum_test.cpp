#include "solver_conundrum_test.hpp"

#include "solver_test_utils.hpp"

#include <QString>
#include <QStringList>
#include <QTest>
#include <QVariantMap>

using countdown::app::test::canSpell;

void ConundrumSolverTest::solveConundrum_found() {
    // A scrambled anagram of "countdown", the unique 9-letter anagram of
    // these letters in the bundled dictionary.
    const QString rack = QStringLiteral("wodnuocnt");
    const QVariantMap result = solver_.solveConundrum(rack);

    QCOMPARE(result["found"].toBool(), true);
    const QStringList answers = result["answers"].toStringList();
    QCOMPARE(answers, QStringList{QStringLiteral("countdown")});
    for (const QString& answer : answers) {
        QCOMPARE(answer.size(), rack.size());
        QVERIFY(canSpell(rack, answer));
    }
}

void ConundrumSolverTest::solveConundrum_notFound() {
    const QVariantMap result = solver_.solveConundrum(QStringLiteral("zzzzzzzzz"));

    QCOMPARE(result["found"].toBool(), false);
    QVERIFY(result["answers"].toStringList().isEmpty());
}

void ConundrumSolverTest::solveConundrum_emptyInput() {
    // Distinct from solveConundrum_notFound above: an empty rack fails
    // find_matches() with SolveError::empty_input rather than no_solution.
    const QVariantMap result = solver_.solveConundrum(QStringLiteral(""));

    QCOMPARE(result["found"].toBool(), false);
    QVERIFY(result["answers"].toStringList().isEmpty());
}
