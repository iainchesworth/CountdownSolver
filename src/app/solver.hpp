#pragma once

#include <countdown/letters/dictionary.hpp>

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

#include <optional>
#include <random>

namespace countdown::app {

// The QML-facing backend. Registered as the context property `solver`
// (see main.cpp); every Q_INVOKABLE returns a QVariantMap so QML reads it as a
// plain JS object. All game logic is delegated to countdown::solver.
//
// Return shapes (consumed verbatim by the QML) are documented per method.
class Solver : public QObject {
    Q_OBJECT

public:
    explicit Solver(QObject* parent = nullptr);

    // { "value": int, "diff": int, "exact": bool, "steps": [QString] }
    Q_INVOKABLE QVariantMap solveNumbers(const QVariantList& numbers, int target) const;

    // { "total": int, "shown": int, "maxLen": int, "longest": [QString],
    //   "groups": [ { "len": int, "count": int, "words": [QString] } ] }
    // "count" is the true number of words of that length across every match;
    // "words" may hold fewer if maxResults truncated the rendered list.
    Q_INVOKABLE QVariantMap solveLetters(const QString& rack, int minLen, int maxResults) const;

    // { "found": bool, "answers": [QString] }
    Q_INVOKABLE QVariantMap solveConundrum(const QString& letters) const;

    // Nine letters for the letters game, drawn from a Scrabble-weighted tile
    // pool respecting Countdown's 3v/6c, 4v/5c or 5v/4c vowel-consonant
    // split - independent of the active dictionary, same as the real show.
    Q_INVOKABLE QString randomRack() const;
    // Nine scrambled letters of a real word; "" if unavailable.
    Q_INVOKABLE QString randomConundrum() const;

    // Version + git provenance, for an About/Settings display.
    Q_INVOKABLE QString versionDetails() const;
    // Bare semantic version ("0.1.0"), for compact display (e.g. the sidebar footer).
    Q_INVOKABLE QString shortVersion() const;
    // True when the working tree had uncommitted changes at build time.
    Q_INVOKABLE bool isDirty() const;

    // True once a full word list has been found at <config-dir>/words.txt.
    Q_INVOKABLE bool fullDictionaryAvailable() const;
    // True when solves are drawing from the full list rather than the default.
    Q_INVOKABLE bool usingFullDictionary() const;
    // Switches the active word list. Returns false (mode left unchanged) if
    // `full` is requested but no full list was found.
    Q_INVOKABLE bool setUseFullDictionary(bool full);

    // Word count of whichever dictionary is currently active, for display
    // (e.g. the sidebar footer).
    Q_INVOKABLE int dictionaryWordCount() const;

private:
    [[nodiscard]] QString shuffledWord(std::size_t length) const;
    [[nodiscard]] const letters::Dictionary& active_dictionary() const;

    letters::Dictionary default_dictionary_;
    std::optional<letters::Dictionary> full_dictionary_;
    bool using_full_dictionary_ = false;
    mutable std::mt19937 rng_;
};

}  // namespace countdown::app
