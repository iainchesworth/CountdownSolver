#include "solver_dictionary_test.hpp"

#include "solver.hpp"

#include <QTest>

#ifdef Q_OS_LINUX
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <QTemporaryDir>
#endif

using countdown::app::Solver;

void DictionaryTest::dictionaryWordCount_matchesActiveDictionary() {
    Solver solver;

    const int defaultCount = solver.dictionaryWordCount();
    QVERIFY(defaultCount > 0);

    if (solver.fullDictionaryAvailable()) {
        QVERIFY(solver.setUseFullDictionary(true));
        QVERIFY(solver.dictionaryWordCount() > 0);
    }
}

void DictionaryTest::fullDictionary_defaultInvariant() {
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
void DictionaryTest::fullDictionary_foundOnDisk() {
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

void DictionaryTest::fullDictionary_foundButEmpty() {
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
#endif
