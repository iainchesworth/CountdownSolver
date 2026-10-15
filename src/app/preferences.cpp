#include "preferences.hpp"

#include <QSettings>

#include <algorithm>
#include <array>

namespace countdown::app {
namespace {

constexpr auto kThemeKey = "appearance/theme";
constexpr auto kMinLenKey = "solver/minLen";
constexpr auto kMaxResultsKey = "solver/maxResults";
constexpr auto kFlagInexactKey = "solver/flagInexact";

constexpr auto kDefaultTheme = "system";
constexpr std::array<const char*, 3> kThemes{{"light", "dark", "system"}};

[[nodiscard]] bool is_valid_theme(const QString& value) {
    return std::ranges::any_of(
        kThemes, [&value](const char* theme) { return value == QLatin1String(theme); });
}

[[nodiscard]] bool is_valid_min_len(int value) {
    return value >= Preferences::kMinLenLow && value <= Preferences::kMinLenHigh;
}

[[nodiscard]] bool is_valid_max_results(int value) {
    return value >= Preferences::kMaxResultsLow && value <= Preferences::kMaxResultsHigh;
}

// Reads `key` as an int, or returns `fallback` if it's absent, isn't a number,
// or fails `is_valid` - a saved value that no longer makes sense is treated as
// "never saved", not as an error.
template <typename Validator>
[[nodiscard]] int read_int(const QSettings& settings, const char* key, int fallback,
                           Validator is_valid) {
    bool ok = false;
    const int value = settings.value(QLatin1String(key)).toInt(&ok);
    return (ok && is_valid(value)) ? value : fallback;
}

void store(const char* key, const QVariant& value) {
    QSettings settings;
    settings.setValue(QLatin1String(key), value);
}

}  // namespace

Preferences::Preferences(QObject* parent) : QObject(parent) {
    const QSettings settings;

    const QString saved_theme = settings.value(QLatin1String(kThemeKey)).toString();
    theme_ = is_valid_theme(saved_theme) ? saved_theme : QString::fromLatin1(kDefaultTheme);

    min_len_ = read_int(settings, kMinLenKey, kDefaultMinLen, is_valid_min_len);
    max_results_ = read_int(settings, kMaxResultsKey, kDefaultMaxResults, is_valid_max_results);

    // toBool() maps an absent key (an invalid QVariant) to false, so the
    // default has to be passed through value() itself.
    flag_inexact_ = settings.value(QLatin1String(kFlagInexactKey), true).toBool();
}

void Preferences::setTheme(const QString& value) {
    if (!is_valid_theme(value) || value == theme_) {
        return;
    }
    theme_ = value;
    store(kThemeKey, theme_);
    emit themeChanged();
}

void Preferences::setMinLen(int value) {
    if (!is_valid_min_len(value) || value == min_len_) {
        return;
    }
    min_len_ = value;
    store(kMinLenKey, min_len_);
    emit minLenChanged();
}

void Preferences::setMaxResults(int value) {
    if (!is_valid_max_results(value) || value == max_results_) {
        return;
    }
    max_results_ = value;
    store(kMaxResultsKey, max_results_);
    emit maxResultsChanged();
}

void Preferences::setFlagInexact(bool value) {
    if (value == flag_inexact_) {
        return;
    }
    flag_inexact_ = value;
    store(kFlagInexactKey, flag_inexact_);
    emit flagInexactChanged();
}

}  // namespace countdown::app
