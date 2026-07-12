#pragma once

#include <countdown/error.hpp>
#include <countdown/letters/dictionary.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace countdown::letters {

// Models one round of the Countdown letters game against a given Dictionary.
// Uses the same "deducing this" fluent-builder style as NumbersGame so the two
// game APIs read consistently from the application layer.
class LettersGame {
public:
    explicit LettersGame(const Dictionary& dictionary) noexcept
        : dictionary_(&dictionary) {}

    template <typename Self>
    [[nodiscard]] auto&& with_letters(this Self&& self, std::string_view letters) {
        self.letters_.assign(letters.begin(), letters.end());
        return std::forward<Self>(self);
    }

    template <typename Self>
    [[nodiscard]] auto&& add_letter(this Self&& self, char letter) {
        self.letters_.push_back(letter);
        return std::forward<Self>(self);
    }

    [[nodiscard]] Result<std::vector<std::string>> solve() const {
        return dictionary_->best_words(letters_);
    }

    [[nodiscard]] const std::string& letters() const noexcept { return letters_; }

private:
    const Dictionary* dictionary_;
    std::string letters_;
};

}  // namespace countdown::letters
