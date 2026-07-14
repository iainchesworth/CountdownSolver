#pragma once

#include <countdown/error.hpp>
#include <countdown/letters/alphabet.hpp>
#include <countdown/letters/dictionary.hpp>
#include <countdown/letters/frequencies.hpp>

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace countdown::conundrum {

// Models the Countdown conundrum round: find the single word that is a full
// anagram of a scrambled letter rack. Uses the same "deducing this"
// fluent-builder style as NumbersGame/LettersGame so all three game APIs read
// consistently from the application layer.
class ConundrumGame {
public:
    // Stores a pointer to `dictionary` rather than a copy, so the caller must
    // ensure `dictionary` outlives this ConundrumGame; constructing from a
    // temporary or shorter-lived Dictionary leaves a dangling reference.
    explicit ConundrumGame(const letters::Dictionary& dictionary) noexcept
        : dictionary_(&dictionary) {}

    template <typename Self>
    [[nodiscard]] auto&& with_letters(this Self&& self, std::string_view letters) {
        self.letters_.assign(letters.begin(), letters.end());
        return std::forward<Self>(self);
    }

    // Rejects a rack containing any codepoint the dictionary's own Alphabet
    // doesn't recognise; an empty rack is left to Dictionary::find_matches,
    // which reports empty_input.
    [[nodiscard]] Result<void> validate() const {
        for (const char32_t codepoint : letters::decode_utf8(letters_)) {
            if (dictionary_->alphabet().fold(codepoint).count == 0) {
                return std::unexpected(SolveError::invalid_letter);
            }
        }
        return {};
    }

    // A full anagram is a match whose length equals the rack's own folded
    // letter count - NOT letters_.size() (a byte count once the rack can
    // contain multi-byte UTF-8) - so min_length restricts find_matches to
    // exactly that.
    [[nodiscard]] Result<std::vector<std::string>> solve() const {
        return validate().and_then([this] {
            const auto rack_length = static_cast<std::size_t>(
                letters::letter_count(letters::frequencies_of(letters_, dictionary_->alphabet())));
            return dictionary_->find_matches(letters_, rack_length);
        });
    }

    [[nodiscard]] const std::string& letters() const noexcept { return letters_; }

private:
    const letters::Dictionary* dictionary_;
    std::string letters_;
};

}  // namespace countdown::conundrum
