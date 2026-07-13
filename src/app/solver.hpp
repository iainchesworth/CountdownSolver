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

    // { "total": int, "shown": int, "maxLen": int,
    //   "longest": [QString], "groups": [ { "len": int, "words": [QString] } ] }
    Q_INVOKABLE QVariantMap solveLetters(const QString& rack, int minLen, int maxResults) const;

    // { "found": bool, "answers": [QString] }
    Q_INVOKABLE QVariantMap solveConundrum(const QString& letters) const;

    // Nine letters for the letters game; "" if unavailable.
    Q_INVOKABLE QString randomRack() const;
    // Nine scrambled letters of a real word; "" if unavailable.
    Q_INVOKABLE QString randomConundrum() const;

    // Version + git provenance, for an About/Settings display.
    Q_INVOKABLE QString versionDetails() const;

private:
    [[nodiscard]] QString shuffledWord(std::size_t length) const;

    std::optional<letters::Dictionary> dictionary_;
    mutable std::mt19937 rng_;
};

}  // namespace countdown::app
