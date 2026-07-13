#include <countdown/letters/dictionary.hpp>

#include <countdown/detail/ranges_compat.hpp>

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <optional>
#include <ranges>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace countdown::letters {
namespace {

// Trims ASCII whitespace and lowercases a candidate word. A word containing any
// interior non-letter (digit, apostrophe, hyphen) is rejected outright.
[[nodiscard]] std::optional<std::string> normalise(std::string_view raw) {
    constexpr auto is_space = [](char c) noexcept {
        return c == ' ' || c == '\t' || c == '\r' || c == '\n' || c == '\f' || c == '\v';
    };

    std::size_t begin = 0;
    std::size_t end = raw.size();
    while (begin < end && is_space(raw[begin])) {
        ++begin;
    }
    while (end > begin && is_space(raw[end - 1])) {
        --end;
    }

    const std::string_view trimmed = raw.substr(begin, end - begin);
    if (trimmed.empty()) {
        return std::nullopt;
    }

    std::string word;
    word.reserve(trimmed.size());
    for (const char c : trimmed) {
        const int index = letter_index(c);
        if (index < 0) {
            return std::nullopt;
        }
        word.push_back(static_cast<char>('a' + index));
    }
    return word;
}

}  // namespace

Dictionary Dictionary::from_words(const std::vector<std::string>& words) {
    Dictionary dictionary;
    for (const std::string& raw : words) {
        if (std::optional<std::string> word = normalise(raw)) {
            dictionary.words_.push_back(*std::move(word));
        }
    }

    // Sort and de-duplicate so results are deterministic.
    std::ranges::sort(dictionary.words_);
    const auto duplicates = std::ranges::unique(dictionary.words_);
    dictionary.words_.erase(duplicates.begin(), duplicates.end());

    // Pre-compute a frequency table per word via a ranges pipeline.
    dictionary.frequencies_ =
        dictionary.words_
        | std::views::transform([](const std::string& word) { return frequencies_of(word); })
        | std::ranges::to<std::vector<Frequencies>>();

    return dictionary;
}

Result<Dictionary> Dictionary::load_from_file(const std::filesystem::path& path) {
    std::ifstream input(path);
    if (!input) {
        return std::unexpected(SolveError::dictionary_not_found);
    }

    std::vector<std::string> raw_words;
    for (std::string line; std::getline(input, line);) {
        raw_words.push_back(std::move(line));
    }

    Dictionary dictionary = from_words(raw_words);
    if (dictionary.empty()) {
        return std::unexpected(SolveError::dictionary_empty);
    }
    return dictionary;
}

Result<std::vector<std::string>> Dictionary::best_words(std::string_view letters) const {
    if (letters.empty()) {
        return std::unexpected(SolveError::empty_input);
    }
    if (words_.empty()) {
        return std::unexpected(SolveError::dictionary_empty);
    }

    const Frequencies have = frequencies_of(letters);

    std::vector<std::string> candidates;
    std::size_t longest = 0;
    // zip walks each word alongside its pre-computed frequency table.
    for (const auto [word, frequency] : std::views::zip(words_, frequencies_)) {
        if (!covers(have, frequency)) {
            continue;
        }
        if (word.size() > longest) {
            longest = word.size();
            candidates.clear();
        }
        if (word.size() == longest) {
            candidates.push_back(word);
        }
    }

    if (candidates.empty()) {
        return std::unexpected(SolveError::no_solution);
    }
    std::ranges::sort(candidates);
    return candidates;
}

Result<std::vector<std::string>> Dictionary::find_matches(std::string_view letters,
                                                          std::size_t min_length) const {
    if (letters.empty()) {
        return std::unexpected(SolveError::empty_input);
    }
    if (words_.empty()) {
        return std::unexpected(SolveError::dictionary_empty);
    }

    const Frequencies have = frequencies_of(letters);

    std::vector<std::string> matches;
    for (const auto [word, frequency] : std::views::zip(words_, frequencies_)) {
        if (word.size() >= min_length && covers(have, frequency)) {
            matches.push_back(word);
        }
    }

    if (matches.empty()) {
        return std::unexpected(SolveError::no_solution);
    }

    // Longest first; alphabetical within a length.
    std::ranges::sort(matches, [](const std::string& lhs, const std::string& rhs) {
        return lhs.size() != rhs.size() ? lhs.size() > rhs.size() : lhs < rhs;
    });
    return matches;
}

std::vector<std::string> Dictionary::words_of_length(std::size_t length) const {
    auto view = words_
        | std::views::filter([length](const std::string& word) { return word.size() == length; });
    return std::ranges::to<std::vector<std::string>>(view);
}

std::vector<std::string> Dictionary::sample(std::size_t step, std::size_t count) const {
    if (step == 0) {
        step = 1;
    }
    // stride selects every step-th word; take caps the result.
    auto view = countdown::detail::stride(words_, static_cast<std::ptrdiff_t>(step))
              | std::views::take(count);
    return std::ranges::to<std::vector<std::string>>(view);
}

}  // namespace countdown::letters
