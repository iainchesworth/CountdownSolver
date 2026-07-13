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
#include <array>

using countdown::app::Solver;

namespace {

// Legal vowel/consonant splits for a 9-letter rack, mirroring
// Solver::randomRack()'s kVowelConsonantSplits (src/app/solver.cpp) - not
// exposed publicly, so duplicated here to check the contract from outside.
constexpr std::array<std::pair<int, int>, 3> kLegalSplits{{{3, 6}, {4, 5}, {5, 4}}};

[[nodiscard]] bool isVowel(QChar ch) {
    switch (ch.toLower().unicode()) {
        case 'a': case 'e': case 'i': case 'o': case 'u': return true;
        default: return false;
    }
}

// True iff every letter in `word` occurs in `rack` at least as often - i.e.
// `word` is genuinely spellable from `rack`'s letters. Used to check the
// solving algorithm's correctness without depending on the exact contents of
// the (large, real) bundled dictionary.
[[nodiscard]] bool canSpell(const QString& rack, const QString& word) {
    std::array<int, 26> counts{};
    for (const QChar ch : rack.toLower()) {
        if (ch.isLetter()) {
            ++counts[static_cast<std::size_t>(ch.unicode() - u'a')];
        }
    }
    for (const QChar ch : word.toLower()) {
        if (!ch.isLetter() || --counts[static_cast<std::size_t>(ch.unicode() - u'a')] < 0) {
            return false;
        }
    }
    return true;
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
    // A letter-rich rack against the real bundled dictionary - exact content
    // isn't asserted (the dictionary can change independently of this test);
    // instead every structural invariant of the response shape is checked.
    const QString rack = QStringLiteral("considerations");
    const QVariantMap result = solver.solveLetters(rack, 1, 10000);

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

void SolverTest::solveLetters_respectsMaxResults() {
    const Solver solver;
    const QString rack = QStringLiteral("considerations");
    const QVariantMap full = solver.solveLetters(rack, 1, 1000);
    QVERIFY(full["total"].toInt() > 1);

    const QVariantMap capped = solver.solveLetters(rack, 1, 1);
    QCOMPARE(capped["total"].toInt(), full["total"].toInt());
    QCOMPARE(capped["shown"].toInt(), 1);

    const QVariantList groups = capped["groups"].toList();
    QCOMPARE(groups.size(), 1);
    QCOMPARE(groups[0].toMap()["words"].toStringList().size(), 1);
}

void SolverTest::solveLetters_noMatches() {
    const Solver solver;
    // No dictionary word is spelled from only x/y/z.
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
    // A scrambled anagram of "countdown", the unique 9-letter anagram of
    // these letters in the bundled dictionary.
    const QString rack = QStringLiteral("wodnuocnt");
    const QVariantMap result = solver.solveConundrum(rack);

    QCOMPARE(result["found"].toBool(), true);
    const QStringList answers = result["answers"].toStringList();
    QCOMPARE(answers, QStringList{QStringLiteral("countdown")});
    for (const QString& answer : answers) {
        QCOMPARE(answer.size(), rack.size());
        QVERIFY(canSpell(rack, answer));
    }
}

void SolverTest::solveConundrum_notFound() {
    const Solver solver;
    const QVariantMap result = solver.solveConundrum(QStringLiteral("zzzzzzzzz"));

    QCOMPARE(result["found"].toBool(), false);
    QVERIFY(result["answers"].toStringList().isEmpty());
}

void SolverTest::randomRack_respectsWeightedTilePool() {
    const Solver solver;
    const QString rack = solver.randomRack();

    QVERIFY(!rack.isEmpty());
    QCOMPARE(rack.size(), 9);
    QCOMPARE(rack, rack.toUpper());

    const int vowels = static_cast<int>(std::count_if(rack.begin(), rack.end(), isVowel));
    const int consonants = static_cast<int>(rack.size()) - vowels;
    const bool isLegalSplit = std::any_of(
        kLegalSplits.begin(), kLegalSplits.end(),
        [&](const auto& split) { return split.first == vowels && split.second == consonants; });
    QVERIFY(isLegalSplit);
}

void SolverTest::randomConundrum_returnsNineLetters() {
    const Solver solver;
    const QString rack = solver.randomConundrum();

    QVERIFY(!rack.isEmpty());
    QCOMPARE(rack.size(), 9);
    QCOMPARE(rack, rack.toUpper());

    // randomConundrum shuffles a real dictionary word, so solving it back
    // must always find at least that word again.
    const QVariantMap result = solver.solveConundrum(rack);
    QCOMPARE(result["found"].toBool(), true);
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

void SolverTest::dictionaryWordCount_matchesActiveDictionary() {
    Solver solver;

    const int defaultCount = solver.dictionaryWordCount();
    QVERIFY(defaultCount > 0);

    if (solver.fullDictionaryAvailable()) {
        QVERIFY(solver.setUseFullDictionary(true));
        QVERIFY(solver.dictionaryWordCount() > 0);
    }
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
