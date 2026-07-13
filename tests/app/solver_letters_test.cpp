#include "solver_letters_test.hpp"

#include "solver_test_utils.hpp"

#include <QString>
#include <QStringList>
#include <QTest>
#include <QVariantList>
#include <QVariantMap>

using countdown::app::test::canSpell;

void LettersSolverTest::solveLetters_groupsByLength() {
    // A letter-rich rack against the real bundled dictionary - exact content
    // isn't asserted (the dictionary can change independently of this test);
    // instead every structural invariant of the response shape is checked.
    const QString rack = QStringLiteral("considerations");
    const QVariantMap result = solver_.solveLetters(rack, 1, 10000);

    const int total = result["total"].toInt();
    QVERIFY(total > 0);
    const int maxLen = result["maxLen"].toInt();
    const QStringList longest = result["longest"].toStringList();
    QVERIFY(!longest.isEmpty());
    for (const QString& word : longest) {
        QCOMPARE(word.size(), maxLen);
        QVERIFY(canSpell(rack, word));
    }

    const QVariantList groups = result["groups"].toList();
    QVERIFY(!groups.isEmpty());
    int previousLen = maxLen + 1;
    int wordsSeen = 0;
    for (const QVariant& groupVariant : groups) {
        const QVariantMap group = groupVariant.toMap();
        const int len = group["len"].toInt();
        // Groups are ordered longest-first.
        QVERIFY(len < previousLen);
        previousLen = len;
        const QStringList words = group["words"].toStringList();
        for (const QString& word : words) {
            QCOMPARE(word.size(), len);
            QVERIFY(canSpell(rack, word));
        }
        wordsSeen += static_cast<int>(words.size());
    }
    // Nothing was truncated: maxResults (1000) comfortably exceeds however
    // many matches "considerations" actually has.
    QCOMPARE(result["shown"].toInt(), total);
    QCOMPARE(wordsSeen, total);
}

void LettersSolverTest::solveLetters_respectsMaxResults() {
    const QString rack = QStringLiteral("considerations");
    const QVariantMap full = solver_.solveLetters(rack, 1, 1000);
    QVERIFY(full["total"].toInt() > 1);

    const QVariantMap capped = solver_.solveLetters(rack, 1, 1);
    QCOMPARE(capped["total"].toInt(), full["total"].toInt());
    QCOMPARE(capped["shown"].toInt(), 1);

    const QVariantList groups = capped["groups"].toList();
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups[0].toMap()["words"].toStringList().size(), 1);
}

void LettersSolverTest::solveLetters_noMatches() {
    // No dictionary word is spelled from only x/y/z.
    const QVariantMap result = solver_.solveLetters(QStringLiteral("xyzxyz"), 1, 10);

    QCOMPARE(result["total"].toInt(), 0);
    QCOMPARE(result["shown"].toInt(), 0);
    QVERIFY(result["longest"].toStringList().isEmpty());
    QVERIFY(result["groups"].toList().isEmpty());

    // Empty rack takes the same early-return path.
    const QVariantMap empty = solver_.solveLetters(QStringLiteral(""), 1, 10);
    QCOMPARE(empty["total"].toInt(), 0);
}
