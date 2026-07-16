#include <countdown/letters/dictionary.hpp>
#include <countdown/letters/letters_game.hpp>

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace {

// Built once and reused across every fuzzer invocation (libFuzzer keeps the
// process alive between runs) - the word list never changes, so rebuilding
// it per input would dominate runtime with no coverage benefit.
const countdown::letters::Dictionary& fuzz_dictionary() {
    static const countdown::letters::Dictionary dictionary =
        countdown::letters::Dictionary::from_words(
            {"a", "an", "and", "cat", "car", "care", "scare", "scarce", "trace",
             "actor", "tractor", "notice", "auction", "creation", "reaction"});
    return dictionary;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    // Countdown racks are 9-10 letters; a much larger input can't reach any
    // code path a real rack doesn't already exercise, so cap it rather than
    // spend fuzzing budget on uninteresting huge-input hangs.
    if (size > 256) {
        return 0;
    }

    const std::string_view letters(reinterpret_cast<const char*>(data), size);
    [[maybe_unused]] const auto result =
        countdown::letters::LettersGame(fuzz_dictionary()).with_letters(letters).solve();
    return 0;
}
