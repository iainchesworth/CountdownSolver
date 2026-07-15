#include <countdown/letters/alphabet.hpp>

#include <cstddef>
#include <cstdint>
#include <string_view>

// decode_utf8() is documented as permissive - it must never crash on
// malformed or truncated input, only skip bytes it can't decode. That
// guarantee is exactly what this harness fuzzes: every rack/word validation
// path in LettersGame/ConundrumGame/Dictionary funnels through here first.
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
    const std::string_view text(reinterpret_cast<const char*>(data), size);
    [[maybe_unused]] const auto codepoints = countdown::letters::decode_utf8(text);
    return 0;
}
