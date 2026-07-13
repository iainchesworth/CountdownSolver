#include "solver_test.hpp"

#include "solver.hpp"

#ifdef Q_OS_LINUX
#include "logging/logging.hpp"
#include "platform/platform.hpp"
#endif

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
    const QVariantMap result = solver_.solveNumbers(QVariantList{50, 50}, 100);

    QCOMPARE(result["exact"].toBool(), true);
    QCOMPARE(result["value"].toLongLong(), 100LL);
    QCOMPARE(result["diff"].toLongLong(), 0LL);
    const QStringList steps = result["steps"].toStringList();
    QVERIFY(!steps.isEmpty());
    QCOMPARE(steps.first(), QStringLiteral("50 + 50 = 100"));
}

void SolverTest::solveNumbers_closestFallback() {
    // {1, 2} can reach at most 3 (1+2); nowhere near target 999, so this
    // exercises the "not exact" branch deterministically.
    const QVariantMap result = solver_.solveNumbers(QVariantList{1, 2}, 999);

    QCOMPARE(result["exact"].toBool(), false);
    QCOMPARE(result["value"].toLongLong(), 3LL);
    QCOMPARE(result["diff"].toLongLong(), 996LL);
}

void SolverTest::solveNumbers_emptyInput() {
    const QVariantMap result = solver_.solveNumbers(QVariantList{}, 500);

    QCOMPARE(result["exact"].toBool(), false);
    QCOMPARE(result["value"].toLongLong(), 0LL);
    QCOMPARE(result["diff"].toLongLong(), 500LL);
    QVERIFY(result["steps"].toStringList().isEmpty());
}

void SolverTest::solveLetters_groupsByLength() {
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

void SolverTest::solveLetters_respectsMaxResults() {
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

void SolverTest::solveLetters_noMatches() {
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

void SolverTest::solveConundrum_found() {
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

void SolverTest::solveConundrum_notFound() {
    const QVariantMap result = solver_.solveConundrum(QStringLiteral("zzzzzzzzz"));

    QCOMPARE(result["found"].toBool(), false);
    QVERIFY(result["answers"].toStringList().isEmpty());
}

void SolverTest::solveConundrum_emptyInput() {
    // Distinct from solveConundrum_notFound above: an empty rack fails
    // find_matches() with SolveError::empty_input rather than no_solution.
    const QVariantMap result = solver_.solveConundrum(QStringLiteral(""));

    QCOMPARE(result["found"].toBool(), false);
    QVERIFY(result["answers"].toStringList().isEmpty());
}

void SolverTest::randomRack_respectsWeightedTilePool() {
    const QString rack = solver_.randomRack();

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
    const QString rack = solver_.randomConundrum();

    QVERIFY(!rack.isEmpty());
    QCOMPARE(rack.size(), 9);
    QCOMPARE(rack, rack.toUpper());

    // randomConundrum shuffles a real dictionary word, so solving it back
    // must always find at least that word again.
    const QVariantMap result = solver_.solveConundrum(rack);
    QCOMPARE(result["found"].toBool(), true);
}

void SolverTest::versionDetails_nonEmpty() {
    const QString details = solver_.versionDetails();

    QVERIFY(!details.isEmpty());
    QVERIFY(details.contains(QStringLiteral("CountdownSolver")));
}

void SolverTest::shortVersion_matchesLibraryVersion() {
    const QString expected = QString::fromUtf8(countdown::version_string.data(),
                                                 static_cast<qsizetype>(countdown::version_string.size()));
    QCOMPARE(solver_.shortVersion(), expected);
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

    // No words.txt exists in this process's config dir (a fresh test/CI
    // environment), so the status message is the "not found" default rather
    // than the "found but empty" variant - see fullDictionary_foundButEmpty
    // for that branch.
    if (!available) {
        QVERIFY(solver.fullDictionaryStatus().contains(QStringLiteral("Add a words.txt")));
    }
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
        QVERIFY(solver.fullDictionaryStatus().contains(QStringLiteral("available")));
    }

    if (previous.isNull()) {
        qunsetenv("XDG_CONFIG_HOME");
    } else {
        qputenv("XDG_CONFIG_HOME", previous);
    }
}

void SolverTest::fullDictionary_foundButEmpty() {
    // Distinct from fullDictionary_foundOnDisk above: every line normalises
    // to nothing (digits aren't letters), so load_from_file() reports
    // dictionary_empty rather than dictionary_not_found.
    QTemporaryDir configDir;
    QVERIFY(configDir.isValid());
    QVERIFY(QDir(configDir.path()).mkpath(QStringLiteral("countdown-solver")));

    QFile wordsFile(configDir.path() + QStringLiteral("/countdown-solver/words.txt"));
    QVERIFY(wordsFile.open(QIODevice::WriteOnly | QIODevice::Text));
    wordsFile.write("123\n456\n789\n");
    wordsFile.close();

    const QByteArray previous = qgetenv("XDG_CONFIG_HOME");
    qputenv("XDG_CONFIG_HOME", configDir.path().toUtf8());

    {
        Solver solver;
        QVERIFY(!solver.fullDictionaryAvailable());
        QVERIFY(solver.fullDictionaryStatus().contains(QStringLiteral("contained no usable words")));
    }

    if (previous.isNull()) {
        qunsetenv("XDG_CONFIG_HOME");
    } else {
        qputenv("XDG_CONFIG_HOME", previous);
    }
}

void SolverTest::logging_installWithoutConfigDirSkipsFileSink() {
    const QByteArray previousXdg = qgetenv("XDG_CONFIG_HOME");
    const QByteArray previousHome = qgetenv("HOME");
    qunsetenv("XDG_CONFIG_HOME");
    qunsetenv("HOME");
    QVERIFY(countdown::platform::current().config_dir.empty());

    // install() must tolerate having nowhere to put a log file - console
    // output alone is still useful, and this must not crash.
    const QtMessageHandler previousHandler = qInstallMessageHandler(nullptr);
    countdown::app::logging::install();
    qCWarning(lcApp) << "console-only message";
    qInstallMessageHandler(previousHandler);

    if (previousXdg.isNull()) {
        qunsetenv("XDG_CONFIG_HOME");
    } else {
        qputenv("XDG_CONFIG_HOME", previousXdg);
    }
    if (previousHome.isNull()) {
        qunsetenv("HOME");
    } else {
        qputenv("HOME", previousHome);
    }
}

void SolverTest::logging_installRotatesAndWritesAllSeverities() {
    QTemporaryDir configDir;
    QVERIFY(configDir.isValid());

    const QByteArray previousXdg = qgetenv("XDG_CONFIG_HOME");
    qputenv("XDG_CONFIG_HOME", configDir.path().toUtf8());

    const QString logPath =
        configDir.path() + QStringLiteral("/countdown-solver/logs/countdown-solver.log");
    const QtMessageHandler previousHandler = qInstallMessageHandler(nullptr);

    // 1st install(): no log file exists yet - exercises rotate_if_oversized()'s
    // "doesn't exist" early-return branch, then creates the file fresh.
    countdown::app::logging::install();
    qCDebug(lcApp) << "debug message";
    qCInfo(lcQml) << "info message";
    qCWarning(lcSolver) << "warning message";
    qCCritical(lcDictionary) << "critical message";
    QVERIFY(QFile::exists(logPath));
    QVERIFY(!QFile::exists(logPath + QStringLiteral(".1")));

    // 2nd install(): the file now exists but is tiny - exercises the "exists
    // but under the rotation threshold" early-return branch.
    countdown::app::logging::install();
    QVERIFY(!QFile::exists(logPath + QStringLiteral(".1")));

    // Grow the file past the rotation threshold, then install() a 3rd time -
    // exercises the actual rotation (rename to a ".1" backup).
    {
        QFile log(logPath);
        QVERIFY(log.open(QIODevice::Append));
        log.write(QByteArray(1'100'000, 'x'));
    }
    countdown::app::logging::install();
    QVERIFY(QFile::exists(logPath + QStringLiteral(".1")));

    qCWarning(lcApp) << "post-rotation message";
    qInstallMessageHandler(previousHandler);

    QFile log(logPath);
    QVERIFY(log.open(QIODevice::ReadOnly | QIODevice::Text));
    const QString contents = QString::fromUtf8(log.readAll());
    QVERIFY(contents.contains(QStringLiteral("post-rotation message")));

    if (previousXdg.isNull()) {
        qunsetenv("XDG_CONFIG_HOME");
    } else {
        qputenv("XDG_CONFIG_HOME", previousXdg);
    }
}
#endif

QTEST_APPLESS_MAIN(SolverTest)
