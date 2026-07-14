#pragma once

#include <countdown/error.hpp>
#include <countdown/letters/alphabet.hpp>
#include <countdown/letters/frequencies.hpp>

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace countdown::letters {

// A word list, stored alongside a pre-computed letter frequency table for each
// word so that solving a round is a single linear scan.
class Dictionary {
public:
    // Builds a dictionary from an in-memory word list, folded through
    // `alphabet` (English by default, matching every call site that existed
    // before Alphabet did). A word containing any codepoint `alphabet`
    // rejects is discarded; the original UTF-8 spelling is kept for display
    // (e.g. "café", not "cafe") while matching/length always use the folded
    // form (see Frequencies/letter_count in frequencies.hpp).
    [[nodiscard]] static Dictionary from_words(const std::vector<std::string>& words,
                                                const Alphabet& alphabet = english_alphabet());

    // Loads a newline-delimited UTF-8 word list from disk. Returns a
    // SolveError instead of throwing if the file is missing or yields no
    // usable words.
    [[nodiscard]] static Result<Dictionary> load_from_file(
        const std::filesystem::path& path, const Alphabet& alphabet = english_alphabet());

    // Returns every longest word that can be spelled from `letters`
    // (respecting letter multiplicities), sorted alphabetically.
    [[nodiscard]] Result<std::vector<std::string>> best_words(std::string_view letters) const;

    // Returns every word of length >= min_length that can be spelled from
    // `letters`, ordered by descending length then alphabetically. A word whose
    // length equals letters.size() is a full anagram of the rack.
    [[nodiscard]] Result<std::vector<std::string>> find_matches(std::string_view letters,
                                                                std::size_t min_length = 1) const;

    [[nodiscard]] std::size_t size() const noexcept { return words_.size(); }
    [[nodiscard]] bool empty() const noexcept { return words_.empty(); }

    // All dictionary words of exactly `length`, in alphabetical order.
    [[nodiscard]] std::vector<std::string> words_of_length(std::size_t length) const;

    // Returns a sample of the word list: every `step`-th word, up to `count`
    // words. A lightweight demonstration/diagnostic helper built from
    // std::views::stride.
    [[nodiscard]] std::vector<std::string> sample(std::size_t step, std::size_t count) const;

    // The Alphabet this dictionary was built with. LettersGame/ConundrumGame
    // use it to validate rack input consistently with how this dictionary's
    // own words were folded (e.g. a French dictionary's rack should accept
    // "é", not just plain ASCII).
    [[nodiscard]] const Alphabet& alphabet() const noexcept { return alphabet_; }

private:
    Dictionary() = default;

    Alphabet alphabet_ = english_alphabet();
    std::vector<std::string> words_;        // original UTF-8 spelling, for display
    std::vector<Frequencies> frequencies_;  // folded via alphabet_, for matching/length
};

}  // namespace countdown::letters
