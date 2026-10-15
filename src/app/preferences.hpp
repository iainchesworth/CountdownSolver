#pragma once

#include <QObject>
#include <QString>

namespace countdown::app {

// The preferences that survive a restart, other than the display language
// (which LanguageManager owns): the theme and the Settings page's solver
// options. Exposed to QML as the `preferences` context property; Theme.qml and
// AppState.qml seed themselves from it and write every change straight back.
//
// Everything is read from QSettings once, in the constructor. main() builds
// this before it loads any QML, so the singletons start out holding the saved
// values and the first frame is already the right theme - there is no frame
// of the default theme to flash through.
//
// Every setter validates (an unknown theme, an out-of-range number) and keeps
// the previous value rather than storing something the UI can't represent, so
// a hand-edited settings file can never wedge the app.
//
// Keys follow the "group/name" style `language/code` already uses:
// appearance/theme, solver/minLen, solver/maxResults, solver/flagInexact.
//
// The "use the full dictionary" choice is deliberately not persisted: it's
// only meaningful once the deferred dictionary load has finished and a
// custom words.txt turned out to exist, so restoring it belongs with
// Solver's load sequence, not here.
class Preferences : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString theme READ theme WRITE setTheme NOTIFY themeChanged)
    Q_PROPERTY(int minLen READ minLen WRITE setMinLen NOTIFY minLenChanged)
    Q_PROPERTY(int maxResults READ maxResults WRITE setMaxResults NOTIFY maxResultsChanged)
    Q_PROPERTY(bool flagInexact READ flagInexact WRITE setFlagInexact NOTIFY flagInexactChanged)

public:
    // The defaults a fresh install starts with (also what AppState.qml and
    // Theme.qml fall back to when no Preferences is registered at all).
    static constexpr int kDefaultMinLen = 4;
    static constexpr int kMinLenLow = 3;
    static constexpr int kMinLenHigh = 5;
    static constexpr int kDefaultMaxResults = 60;
    static constexpr int kMaxResultsLow = 20;
    static constexpr int kMaxResultsHigh = 150;

    explicit Preferences(QObject* parent = nullptr);

    // "light" | "dark" | "system".
    [[nodiscard]] QString theme() const { return theme_; }
    [[nodiscard]] int minLen() const noexcept { return min_len_; }
    [[nodiscard]] int maxResults() const noexcept { return max_results_; }
    [[nodiscard]] bool flagInexact() const noexcept { return flag_inexact_; }

    // Each returns without changing anything if `value` isn't valid, or is
    // already the current value.
    void setTheme(const QString& value);
    void setMinLen(int value);
    void setMaxResults(int value);
    void setFlagInexact(bool value);

signals:
    void themeChanged();
    void minLenChanged();
    void maxResultsChanged();
    void flagInexactChanged();

private:
    QString theme_;
    int min_len_;
    int max_results_;
    bool flag_inexact_;
};

}  // namespace countdown::app
