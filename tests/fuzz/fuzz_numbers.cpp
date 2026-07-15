#include <countdown/numbers/numbers_game.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <vector>

namespace {

std::int32_t read_int32(const std::uint8_t* data) {
    std::int32_t value = 0;
    std::memcpy(&value, data, sizeof(value));
    return value;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    constexpr std::size_t kIntSize = sizeof(std::int32_t);

    // A target plus at least one number - anything shorter can't even fill
    // the target.
    if (size < kIntSize) {
        return 0;
    }

    const int target = read_int32(data);
    data += kIntSize;
    size -= kIntSize;

    // A real numbers round draws exactly six tiles; more couldn't exercise
    // anything the solver's search doesn't already cover.
    constexpr std::size_t kMaxNumbers = 6;
    std::vector<int> numbers;
    while (size >= kIntSize && numbers.size() < kMaxNumbers) {
        numbers.push_back(read_int32(data));
        data += kIntSize;
        size -= kIntSize;
    }

    [[maybe_unused]] const auto result =
        countdown::numbers::NumbersGame{}.with_target(target).with_numbers(numbers).solve();
    return 0;
}
