#pragma once

#include <QChar>
#include <QString>

#include <array>

namespace countdown::app::test {

// True iff every letter in `word` occurs in `rack` at least as often - i.e.
// `word` is genuinely spellable from `rack`'s letters. Used to check the
// solving algorithm's correctness without depending on the exact contents of
// the (large, real) bundled dictionary. Shared by the letters- and
// conundrum-solving test files, which both check this property against
// answers pulled from the real dictionary.
[[nodiscard]] inline bool canSpell(const QString& rack, const QString& word) {
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

}  // namespace countdown::app::test
