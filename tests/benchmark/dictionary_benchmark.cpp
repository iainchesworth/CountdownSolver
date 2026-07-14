#include <countdown/letters/dictionary.hpp>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <string>
#include <vector>

using countdown::letters::Dictionary;

namespace {

// COUNTDOWN_DICTIONARY_PATH points at the real bundled word list (see
// tests/CMakeLists.txt) rather than the small fixture under tests/data/ -
// the whole point is to measure the same dictionary every Solver actually
// loads. This is what tests/app/solver_test.cpp was rebuilding once per test
// slot before being fixed to share a single Solver: see that file's history
// for why constructing this is worth keeping an eye on.
[[nodiscard]] std::vector<std::string> load_bundled_words() {
    std::ifstream input(COUNTDOWN_DICTIONARY_PATH);
    std::vector<std::string> words;
    for (std::string line; std::getline(input, line);) {
        words.push_back(std::move(line));
    }
    return words;
}

}  // namespace

TEST_CASE("Dictionary performance", "[dictionary][benchmark]") {
    const std::vector<std::string> words = load_bundled_words();
    REQUIRE(!words.empty());

    BENCHMARK("from_words (full bundled dictionary)") {
        return Dictionary::from_words(words);
    };

    const Dictionary dictionary = Dictionary::from_words(words);

    BENCHMARK("find_matches (letters-game rack)") {
        return dictionary.find_matches("considerations", 1);
    };

    BENCHMARK("best_words (longest-word rack)") {
        return dictionary.best_words("considerations");
    };
}
