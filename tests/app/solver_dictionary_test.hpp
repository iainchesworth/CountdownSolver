#pragma once

#include <QObject>

// Solver's full-dictionary discovery/status contract. Unlike the other
// Solver test classes, every slot here builds its own local Solver rather
// than sharing app_tests_main.cpp's instance, because these tests exercise
// construction-time behaviour itself (what a fresh Solver discovers on
// disk), which a pre-built shared instance can't retroactively exhibit.
class DictionaryTest : public QObject {
    Q_OBJECT

private slots:
    void dictionaryWordCount_matchesActiveDictionary();
    void fullDictionary_defaultInvariant();
#ifdef Q_OS_LINUX
    void fullDictionary_foundOnDisk();
    void fullDictionary_foundButEmpty();
#endif
};
