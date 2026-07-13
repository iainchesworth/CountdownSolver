#pragma once

#include <countdown/error.hpp>
#include <countdown/letters/dictionary.hpp>
#include <countdown/letters/frequencies.hpp>

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
    // Stores a pointer to `dictionary` rather than a copy, so the caller must
    // ensure `dictionary` outlives this LettersGame; constructing from a
    // temporary or shorter-lived Dictionary leaves a dangling reference.
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

    // Rejects a rack containing any non-alphabetic character; an empty rack
    // is left to Dictionary::best_words, which reports empty_input.
    [[nodiscard]] Result<void> validate() const {
        for (const char c : letters_) {
            if (letter_index(c) < 0) {
                return std::unexpected(SolveError::invalid_letter);
            }
        }
        return {};
    }

    [[nodiscard]] Result<std::vector<std::string>> solve() const {
        return validate().and_then([this] { return dictionary_->best_words(letters_); });
    }

    [[nodiscard]] const std::string& letters() const noexcept { return letters_; }

private:
    const Dictionary* dictionary_;
    std::string letters_;
};

}  // namespace countdown::letters
