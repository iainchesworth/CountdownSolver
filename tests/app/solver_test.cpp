#include "solver_test.hpp"

#include "solver.hpp"

#include <countdown/version.hpp>

#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QString>
#include <QStringList>
#include <QTemporaryDir>
#include <QTest>
#include <QVariantList>
#include <QVariantMap>

#include <algorithm>

using countdown::app::Solver;

namespace {

// Nine-letter words present in Solver's built-in fallback dictionary
// (see kFallbackWords in src/app/solver.cpp) - used to check randomRack()/
// randomConundrum() invariants without depending on the RNG's exact draw.
const QStringList kNineLetterFallbackWords = {
    QStringLiteral("countdown"), QStringLiteral("conundrum"), QStringLiteral("rectangle"),
    QStringLiteral("cratering"), QStringLiteral("consonant"), QStringLiteral("operation"),
};

[[nodiscard]] QString sortedLower(const QString& text) {
    QString lowered = text.toLower();
    std::sort(lowered.begin(), lowered.end());
    return lowered;
}

}  // namespace

void SolverTest::solveNumbers_exact() {
    const Solver solver;
    const QVariantMap result = solver.solveNumbers(QVariantList{50, 50}, 100);

    QCOMPARE(result["exact"].toBool(), true);
    QCOMPARE(result["value"].toLongLong(), 100LL);
    QCOMPARE(result["diff"].toLongLong(), 0LL);
    const QStringList steps = result["steps"].toStringList();
    QVERIFY(!steps.isEmpty());
    QCOMPARE(steps.first(), QStringLiteral("50 + 50 = 100"));
}

void SolverTest::solveNumbers_closestFallback() {
    const Solver solver;
    // {1, 2} can reach at most 3 (1+2); nowhere near target 999, so this
    // exercises the "not exact" branch deterministically.
    const QVariantMap result = solver.solveNumbers(QVariantList{1, 2}, 999);

    QCOMPARE(result["exact"].toBool(), false);
    QCOMPARE(result["value"].toLongLong(), 3LL);
    QCOMPARE(result["diff"].toLongLong(), 996LL);
}

void SolverTest::solveNumbers_emptyInput() {
    const Solver solver;
    const QVariantMap result = solver.solveNumbers(QVariantList{}, 500);

    QCOMPARE(result["exact"].toBool(), false);
    QCOMPARE(result["value"].toLongLong(), 0LL);
    QCOMPARE(result["diff"].toLongLong(), 500LL);
    QVERIFY(result["steps"].toStringList().isEmpty());
}

void SolverTest::solveLetters_groupsByLength() {
    const Solver solver;
    // Covers exactly two fallback words: "notice" (6) and "vowel" (5).
    const QVariantMap result = solver.solveLetters(QStringLiteral("noticevowel"), 1, 10);

    QCOMPARE(result["total"].toInt(), 2);
    QCOMPARE(result["shown"].toInt(), 2);
    QCOMPARE(result["maxLen"].toInt(), 6);
    QCOMPARE(result["longest"].toStringList(), QStringList{QStringLiteral("notice")});

    const QVariantList groups = result["groups"].toList();
    QCOMPARE(groups.size(), 2);
    QCOMPARE(groups[0].toMap()["len"].toInt(), 6);
    QCOMPARE(groups[0].toMap()["words"].toStringList(), QStringList{QStringLiteral("notice")});
    QCOMPARE(groups[1].toMap()["len"].toInt(), 5);
    QCOMPARE(groups[1].toMap()["words"].toStringList(), QStringList{QStringLiteral("vowel")});
}

void SolverTest::solveLetters_respectsMaxResults() {
    const Solver solver;
    const QVariantMap result = solver.solveLetters(QStringLiteral("noticevowel"), 1, 1);

    QCOMPARE(result["total"].toInt(), 2);
    QCOMPARE(result["shown"].toInt(), 1);
    const QVariantList groups = result["groups"].toList();
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups[0].toMap()["words"].toStringList(), QStringList{QStringLiteral("notice")});
}

void SolverTest::solveLetters_noMatches() {
    const Solver solver;
    const QVariantMap result = solver.solveLetters(QStringLiteral("xyzxyz"), 1, 10);

    QCOMPARE(result["total"].toInt(), 0);
    QCOMPARE(result["shown"].toInt(), 0);
    QVERIFY(result["longest"].toStringList().isEmpty());
    QVERIFY(result["groups"].toList().isEmpty());

    // Empty rack takes the same early-return path.
    const QVariantMap empty = solver.solveLetters(QStringLiteral(""), 1, 10);
    QCOMPARE(empty["total"].toInt(), 0);
}

void SolverTest::solveConundrum_found() {
    const Solver solver;
    // An anagram of the fallback word "notice".
    const QVariantMap result = solver.solveConundrum(QStringLiteral("ecitno"));

    QCOMPARE(result["found"].toBool(), true);
    QCOMPARE(result["answers"].toStringList(), QStringList{QStringLiteral("notice")});
}

void SolverTest::solveConundrum_notFound() {
    const Solver solver;
    const QVariantMap result = solver.solveConundrum(QStringLiteral("xyzxyz"));

    QCOMPARE(result["found"].toBool(), false);
    QVERIFY(result["answers"].toStringList().isEmpty());
}

void SolverTest::randomRack_returnsARealFallbackWordsLetters() {
    const Solver solver;
    const QString rack = solver.randomRack();

    QVERIFY(!rack.isEmpty());
    QCOMPARE(rack.size(), 9);
    QCOMPARE(rack, rack.toUpper());

    const QString sorted = sortedLower(rack);
    const bool matchesAFallbackWord = std::any_of(
        kNineLetterFallbackWords.begin(), kNineLetterFallbackWords.end(),
        [&](const QString& word) { return sortedLower(word) == sorted; });
    QVERIFY(matchesAFallbackWord);
}

void SolverTest::randomConundrum_returnsNineLetters() {
    const Solver solver;
    const QString rack = solver.randomConundrum();

    QVERIFY(!rack.isEmpty());
    QCOMPARE(rack.size(), 9);
    QCOMPARE(rack, rack.toUpper());
}

void SolverTest::versionDetails_nonEmpty() {
    const Solver solver;
    const QString details = solver.versionDetails();

    QVERIFY(!details.isEmpty());
    QVERIFY(details.contains(QStringLiteral("CountdownSolver")));
}

void SolverTest::shortVersion_matchesLibraryVersion() {
    const Solver solver;
    const QString expected = QString::fromUtf8(countdown::version_string.data(),
                                                 static_cast<qsizetype>(countdown::version_string.size()));
    QCOMPARE(solver.shortVersion(), expected);
}

void SolverTest::fullDictionary_defaultInvariant() {
    Solver solver;

    QVERIFY(!solver.usingFullDictionary());
    QCOMPARE(solver.setUseFullDictionary(false), true);
    QVERIFY(!solver.usingFullDictionary());

    // setUseFullDictionary(true) succeeds iff a full dictionary was found;
    // this holds regardless of whether the test environment happens to have
    // one on disk.
    const bool available = solver.fullDictionaryAvailable();
    QCOMPARE(solver.setUseFullDictionary(true), available);
    QCOMPARE(solver.usingFullDictionary(), available);
}

#ifdef Q_OS_LINUX
void SolverTest::fullDictionary_foundOnDisk() {
    QTemporaryDir configDir;
    QVERIFY(configDir.isValid());
    QVERIFY(QDir(configDir.path()).mkpath(QStringLiteral("countdown-solver")));

    QFile wordsFile(configDir.path() + QStringLiteral("/countdown-solver/words.txt"));
    QVERIFY(wordsFile.open(QIODevice::WriteOnly | QIODevice::Text));
    wordsFile.write("apple\nbanana\ncherry\n");
    wordsFile.close();

    const QByteArray previous = qgetenv("XDG_CONFIG_HOME");
    qputenv("XDG_CONFIG_HOME", configDir.path().toUtf8());

    {
        // platform::current() (and so the full dictionary) is only read at
        // construction time, so this Solver must be built after the env var
        // is set.
        Solver solver;
        QVERIFY(solver.fullDictionaryAvailable());
        QCOMPARE(solver.setUseFullDictionary(true), true);
        QVERIFY(solver.usingFullDictionary());
    }

    if (previous.isNull()) {
        qunsetenv("XDG_CONFIG_HOME");
    } else {
        qputenv("XDG_CONFIG_HOME", previous);
    }
}
#endif

QTEST_APPLESS_MAIN(SolverTest)
