#pragma once

#include <countdown/error.hpp>
#include <countdown/letters/dictionary.hpp>

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <random>

namespace countdown::app {

// The QML-facing backend. Registered as the context property `solver`
// (see main.cpp); every Q_INVOKABLE returns a QVariantMap so QML reads it as a
// plain JS object. All game logic is delegated to countdown::solver.
//
// Return shapes (consumed verbatim by the QML) are documented per method.
class Solver : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool dictionariesReady READ dictionariesReady NOTIFY dictionariesReadyChanged)

public:
    // Controls when the constructor loads the word lists. kEager (the
    // default, and what every existing call site gets) loads synchronously
    // during construction, same as before this option existed. main.cpp
    // uses kDeferred and calls loadDictionaries() itself once the QML engine
    // has started, so the ~1.1MB bundled word list is parsed after the
    // window is already on screen rather than before it can appear at all.
    enum class DictionaryLoad { kEager, kDeferred };

    explicit Solver(QObject* parent = nullptr, DictionaryLoad load = DictionaryLoad::kEager);

    // Parses and indexes the default (and, if present, the user-supplied
    // full) dictionary using whichever language setLanguageCode() last
    // recorded (English if never called). Safe to call more than once -
    // setLanguageCode() re-calls this itself once dictionaries are already
    // loaded, to reload with the new language's alphabet/word list.
    void loadDictionaries();

    // Records which language's alphabet + bundled word list Solver's game
    // logic should use. If dictionaries have already loaded, reloads them
    // immediately with the new language; otherwise just records the pending
    // choice for the first (deferred) loadDictionaries() call to pick up.
    // Decoupled from LanguageManager on purpose - main.cpp is what connects
    // LanguageManager::currentLanguageChanged to this.
    Q_INVOKABLE void setLanguageCode(const QString& code);

    [[nodiscard]] bool dictionariesReady() const noexcept { return dictionaries_ready_; }

    // { "value": int, "diff": int, "exact": bool, "steps": [QString] }
    Q_INVOKABLE QVariantMap solveNumbers(const QVariantList& numbers, int target) const;

    // Runs solveNumbers() on a worker thread and delivers the result via
    // numbersSolved(), so the potentially-exhaustive search in
    // NumbersGame::search() never blocks the UI thread. QML is expected to
    // show a busy state between the call and the signal and avoid
    // overlapping requests (see NumbersPage.qml).
    Q_INVOKABLE void solveNumbersAsync(const QVariantList& numbers, int target);

    // { "total": int, "shown": int, "maxLen": int, "longest": [QString],
    //   "groups": [ { "len": int, "count": int, "words": [QString] } ] }
    // "count" is the true number of words of that length across every match;
    // "words" may hold fewer if maxResults truncated the rendered list.
    Q_INVOKABLE QVariantMap solveLetters(const QString& rack, int minLen, int maxResults) const;

    // { "found": bool, "answers": [QString] }
    Q_INVOKABLE QVariantMap solveConundrum(const QString& letters) const;

    // A rack of active_dictionary().alphabet().rack_size letters (9 for most
    // languages, 10 for French), drawn from that alphabet's own
    // Scrabble-weighted tile pool respecting its vowel/consonant split where
    // it defines one - independent of the active dictionary's actual words,
    // same as the real show.
    Q_INVOKABLE QString randomRack() const;
    // rackSize() scrambled letters of a real word from the active
    // dictionary; "" if unavailable.
    Q_INVOKABLE QString randomConundrum() const;
    // Letters drawn for a Letters-game rack or Conundrum round in the
    // active language (9 usually, 10 for French) - the single source of
    // truth QML binds against instead of hardcoding 9.
    Q_INVOKABLE int rackSize() const;

    // Version + git provenance, for an About/Settings display.
    Q_INVOKABLE QString versionDetails() const;
    // Bare semantic version ("0.1.0"), for compact display (e.g. the sidebar footer).
    Q_INVOKABLE QString shortVersion() const;
    // True when the working tree had uncommitted changes at build time.
    Q_INVOKABLE bool isDirty() const;
    // Qt runtime version ("6.7.2"), for the third-party licenses display.
    Q_INVOKABLE QString qtVersion() const;

    // True once a full word list has been found at <config-dir>/words.txt.
    Q_INVOKABLE bool fullDictionaryAvailable() const;
    // Human-readable status of the full dictionary, distinguishing "no
    // words.txt found" from "words.txt found but empty/unusable" - for
    // display when fullDictionaryAvailable() is false.
    Q_INVOKABLE QString fullDictionaryStatus() const;
    // True when solves are drawing from the full list rather than the default.
    Q_INVOKABLE bool usingFullDictionary() const;
    // Switches the active word list. Returns false (mode left unchanged) if
    // `full` is requested but no full list was found.
    Q_INVOKABLE bool setUseFullDictionary(bool full);

    // Word count of whichever dictionary is currently active, for display
    // (e.g. the sidebar footer).
    Q_INVOKABLE int dictionaryWordCount() const;

signals:
    // Emitted once, from the main thread, when solveNumbersAsync()'s
    // background computation finishes.
    void numbersSolved(const QVariantMap& result);

    // Emitted once loadDictionaries() finishes populating the word lists.
    void dictionariesReadyChanged();

private:
    [[nodiscard]] QString shuffledWord(std::size_t length) const;
    [[nodiscard]] const letters::Dictionary& active_dictionary() const;

    // Both are placeholder-initialized (an empty dictionary / "not found")
    // at construction and overwritten by loadDictionaries() - see its
    // comment on Solver(). Neither needs to be optional: the placeholder
    // values already mean "no matches"/"not available" while a kDeferred
    // construction is waiting for the real load.
    letters::Dictionary default_dictionary_;
    Result<letters::Dictionary> full_dictionary_;
    bool using_full_dictionary_ = false;
    bool dictionaries_ready_ = false;
    QString language_code_ = QStringLiteral("en");
    mutable std::mt19937 rng_;
};

}  // namespace countdown::app
