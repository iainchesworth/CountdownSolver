#pragma once

#include <countdown/error.hpp>
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
    // Builds a dictionary from an in-memory word list. Words are normalised to
    // lowercase; any word containing a non-letter is discarded.
    [[nodiscard]] static Dictionary from_words(const std::vector<std::string>& words);

    // Loads a newline-delimited word list from disk. Returns a SolveError
    // instead of throwing if the file is missing or yields no usable words.
    [[nodiscard]] static Result<Dictionary> load_from_file(const std::filesystem::path& path);

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

private:
    Dictionary() = default;

    std::vector<std::string> words_;
    std::vector<Frequencies> frequencies_;
};

}  // namespace countdown::letters
