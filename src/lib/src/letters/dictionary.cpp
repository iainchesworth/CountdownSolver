#include <countdown/letters/dictionary.hpp>

#include <countdown/detail/ranges_compat.hpp>
#include <countdown/letters/alphabet.hpp>

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

// Trims ASCII whitespace and lowercases ASCII letters (non-ASCII bytes pass
// through unchanged - real-world word lists are conventionally already
// lowercase, including their accented letters, so this doesn't need a full
// per-codepoint case-folding table). Returns nullopt if the word is empty
// after trimming, or contains any codepoint `alphabet` doesn't recognise at
// all (digit, punctuation, foreign script) - mirroring the previous
// "reject anything not a letter" behaviour, generalized past ASCII.
[[nodiscard]] std::optional<std::string> normalise(std::string_view raw, const Alphabet& alphabet) {
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

    std::string display;
    display.reserve(trimmed.size());
    for (const char c : trimmed) {
        display.push_back((c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c);
    }

    const std::vector<char32_t> codepoints = decode_utf8(display);
    if (codepoints.empty()) {
        return std::nullopt;
    }
    for (const char32_t cp : codepoints) {
        if (alphabet.fold(cp).count == 0) {
            return std::nullopt;
        }
    }

    return display;
}

}  // namespace

Dictionary Dictionary::from_words(const std::vector<std::string>& words, const Alphabet& alphabet) {
    Dictionary dictionary;
    dictionary.alphabet_ = alphabet;
    for (const std::string& raw : words) {
        if (std::optional<std::string> word = normalise(raw, alphabet)) {
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
        | std::views::transform(
              [&alphabet](const std::string& word) { return frequencies_of(word, alphabet); })
        | std::ranges::to<std::vector<Frequencies>>();

    return dictionary;
}

Result<Dictionary> Dictionary::load_from_file(const std::filesystem::path& path,
                                               const Alphabet& alphabet) {
    std::ifstream input(path);
    if (!input) {
        return std::unexpected(SolveError::dictionary_not_found);
    }

    std::vector<std::string> raw_words;
    for (std::string line; std::getline(input, line);) {
        raw_words.push_back(std::move(line));
    }

    Dictionary dictionary = from_words(raw_words, alphabet);
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

    const Frequencies have = frequencies_of(letters, alphabet_);

    std::vector<std::string> candidates;
    int longest = 0;
    // zip walks each word alongside its pre-computed frequency table.
    for (const auto [word, frequency] : std::views::zip(words_, frequencies_)) {
        if (!covers(have, frequency)) {
            continue;
        }
        const int length = letter_count(frequency);
        if (length > longest) {
            longest = length;
            candidates.clear();
        }
        if (length == longest) {
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

    const Frequencies have = frequencies_of(letters, alphabet_);

    // Paired with each match's own folded length so the sort below doesn't
    // need to recompute it per comparison (this dictionary can hold the
    // full ~122k-word bundled English list, so re-deriving length via a
    // fresh frequencies_of() call inside the comparator would be real
    // overhead, not just noise).
    std::vector<std::pair<std::string, int>> matches;
    for (const auto [word, frequency] : std::views::zip(words_, frequencies_)) {
        const int length = letter_count(frequency);
        if (static_cast<std::size_t>(length) >= min_length && covers(have, frequency)) {
            matches.emplace_back(word, length);
        }
    }

    if (matches.empty()) {
        return std::unexpected(SolveError::no_solution);
    }

    // Longest first; alphabetical within a length.
    std::ranges::sort(matches, [](const auto& lhs, const auto& rhs) {
        return lhs.second != rhs.second ? lhs.second > rhs.second : lhs.first < rhs.first;
    });

    return matches
        | std::views::transform([](auto& entry) { return std::move(entry.first); })
        | std::ranges::to<std::vector<std::string>>();
}

std::vector<std::string> Dictionary::words_of_length(std::size_t length) const {
    // words_ is already alphabetically sorted (see from_words()), so a
    // filtered subset of it needs no further sorting.
    std::vector<std::string> result;
    for (const auto [word, frequency] : std::views::zip(words_, frequencies_)) {
        if (static_cast<std::size_t>(letter_count(frequency)) == length) {
            result.push_back(word);
        }
    }
    return result;
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
