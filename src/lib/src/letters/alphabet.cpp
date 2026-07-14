#include <countdown/letters/alphabet.hpp>

#include <cstddef>
#include <initializer_list>
#include <string>

// Every non-ASCII codepoint below is written as a \U0000XXXX universal
// character name (with the glyph itself in a trailing comment) rather than
// as a literal accented character in the source. Literal multi-byte UTF-8
// source characters depend on the compiler correctly detecting the source
// file's encoding to parse a `U'...'` literal as exactly one codepoint;
// MSVC in particular does not reliably do this without extra flags, so
// escapes are the only encoding-agnostic option that behaves identically
// across every compiler in this project's CI matrix (MSVC, Clang, GCC,
// AppleClang).

namespace countdown::letters {
namespace {

// One tile's rack-generation weight: which slot, how many copies in the
// weighted pool, and whether it counts as a vowel for a
// vowel_consonant_splits-enforcing alphabet (ignored otherwise).
struct TileWeight {
    int slot;
    int weight;
    bool vowel;
};

void apply_weights(std::array<int, kMaxAlphabetSize>& weights,
                    std::array<bool, kMaxAlphabetSize>& is_vowel,
                    std::initializer_list<TileWeight> tiles) noexcept {
    for (const TileWeight& tile : tiles) {
        weights[static_cast<std::size_t>(tile.slot)] = tile.weight;
        is_vowel[static_cast<std::size_t>(tile.slot)] = tile.vowel;
    }
}

// Combining marks that decode_utf8() drops rather than emits: Arabic
// tashkeel (fatha/damma/kasra/sukun/shadda/tanwin + superscript alef),
// Hebrew points/cantillation (covers Yiddish's vowel-marking diacritic
// combinations too, since Yiddish uses the same block), and the general
// Unicode combining-diacriticals block (NFD-normalized Latin text, e.g.
// from a clipboard, decomposes an accented letter into base+combining-mark
// instead of the single precomposed codepoint every fold_xxx() above
// expects).
[[nodiscard]] constexpr bool is_combining_mark(char32_t cp) noexcept {
    return (cp >= 0x0300 && cp <= 0x036F) ||   // general combining diacriticals
           (cp >= 0x0591 && cp <= 0x05C7) ||   // Hebrew points/cantillation
           (cp >= 0x064B && cp <= 0x065F) ||   // Arabic tashkeel
           cp == 0x0670;                       // Arabic superscript alef
}

// a, b, c, ..., z - the display form every alphabet's first 26 slots share;
// German/Spanish append their own extra distinct-letter slots after this.
[[nodiscard]] std::vector<std::string> ascii_display_letters() {
    std::vector<std::string> letters;
    letters.reserve(26);
    for (char c = 'a'; c <= 'z'; ++c) {
        letters.emplace_back(1, c);
    }
    return letters;
}

[[nodiscard]] constexpr FoldedLetters none() noexcept {
    return FoldedLetters{};
}

[[nodiscard]] constexpr FoldedLetters one(int index) noexcept {
    return FoldedLetters{{index, 0}, 1};
}

[[nodiscard]] constexpr FoldedLetters two(int first, int second) noexcept {
    return FoldedLetters{{first, second}, 2};
}

// a/A -> 0, ..., z/Z -> 25, or -1 for anything else. Every alphabet's fold
// function falls back to this for the plain (unaccented) Latin letters.
[[nodiscard]] constexpr int ascii_lower_index(char32_t cp) noexcept {
    if (cp >= U'a' && cp <= U'z') {
        return static_cast<int>(cp - U'a');
    }
    if (cp >= U'A' && cp <= U'Z') {
        return static_cast<int>(cp - U'A');
    }
    return -1;
}

// French: 26 slots. Every accented vowel/consonant folds to its base letter
// (French Scrabble tiles carry no accents at all); oe/ae ligatures - which
// also have no tile of their own - expand to their two-letter spelling.
[[nodiscard]] FoldedLetters fold_french(char32_t cp) noexcept {
    const int base = ascii_lower_index(cp);
    if (base >= 0) {
        return one(base);
    }
    switch (cp) {
        case U'\U000000E0':  // à
        case U'\U000000C0':  // À
        case U'\U000000E2':  // â
        case U'\U000000C2':  // Â
        case U'\U000000E1':  // á
        case U'\U000000C1':  // Á
        case U'\U000000E4':  // ä
        case U'\U000000C4':  // Ä
            return one(0);
        case U'\U000000E7':  // ç
        case U'\U000000C7':  // Ç
            return one(2);
        case U'\U000000E9':  // é
        case U'\U000000C9':  // É
        case U'\U000000E8':  // è
        case U'\U000000C8':  // È
        case U'\U000000EA':  // ê
        case U'\U000000CA':  // Ê
        case U'\U000000EB':  // ë
        case U'\U000000CB':  // Ë
            return one(4);
        case U'\U000000EE':  // î
        case U'\U000000CE':  // Î
        case U'\U000000EF':  // ï
        case U'\U000000CF':  // Ï
            return one(8);
        case U'\U000000F4':  // ô
        case U'\U000000D4':  // Ô
        case U'\U000000F6':  // ö
        case U'\U000000D6':  // Ö
            return one(14);
        case U'\U000000F9':  // ù
        case U'\U000000D9':  // Ù
        case U'\U000000FB':  // û
        case U'\U000000DB':  // Û
        case U'\U000000FC':  // ü
        case U'\U000000DC':  // Ü
            return one(20);
        case U'\U000000FF':  // ÿ
        case U'\U00000178':  // Ÿ
            return one(24);
        case U'\U00000153':  // œ
        case U'\U00000152':  // Œ
            return two(14, 4);  // o, e
        case U'\U000000E6':  // æ
        case U'\U000000C6':  // Æ
            return two(0, 4);  // a, e
        default:
            return none();
    }
}

// German: 29 slots (a-z, then A-umlaut=26, O-umlaut=27, U-umlaut=28 as
// their OWN slots - official German Scrabble tiles them separately, never
// folded to A/O/U). Eszett (ss) has no tile in the official game and
// conventionally alternates with "ss", so it expands to two slot-18 ('s')
// entries.
[[nodiscard]] FoldedLetters fold_german(char32_t cp) noexcept {
    const int base = ascii_lower_index(cp);
    if (base >= 0) {
        return one(base);
    }
    switch (cp) {
        case U'\U000000E4':  // ä
        case U'\U000000C4':  // Ä
            return one(26);
        case U'\U000000F6':  // ö
        case U'\U000000D6':  // Ö
            return one(27);
        case U'\U000000FC':  // ü
        case U'\U000000DC':  // Ü
            return one(28);
        case U'\U000000DF':  // ß
            return two(18, 18);  // ss
        default:
            return none();
    }
}

// Spanish: 27 slots (a-z, then N-tilde=26 as its own slot - never folded to
// N). Accented vowels (a-acute e-acute i-acute o-acute u-acute) and the
// u-diaeresis used in gue/gui fold to their base vowel - Spanish Scrabble
// has no separate tiles for these.
[[nodiscard]] FoldedLetters fold_spanish(char32_t cp) noexcept {
    const int base = ascii_lower_index(cp);
    if (base >= 0) {
        return one(base);
    }
    switch (cp) {
        case U'\U000000E1':  // á
        case U'\U000000C1':  // Á
            return one(0);
        case U'\U000000E9':  // é
        case U'\U000000C9':  // É
            return one(4);
        case U'\U000000ED':  // í
        case U'\U000000CD':  // Í
            return one(8);
        case U'\U000000F3':  // ó
        case U'\U000000D3':  // Ó
            return one(14);
        case U'\U000000FA':  // ú
        case U'\U000000DA':  // Ú
        case U'\U000000FC':  // ü
        case U'\U000000DC':  // Ü
            return one(20);
        case U'\U000000F1':  // ñ
        case U'\U000000D1':  // Ñ
            return one(26);
        default:
            return none();
    }
}

// Arabic: 29 slots, 0=alif..27=yaa (traditional order) plus 28=taa marbuta.
// No official adaptation exists to source these folds from (see
// arabic_alphabet()'s doc comment) - hamza-bearing alif variants and the
// standalone hamza all fold to bare alif (slot 0), hamza-on-waw folds to
// waw, and hamza-on-yaa/alif-maksura fold to yaa - all the same base letter
// for word-game purposes, not distinct tiles.
[[nodiscard]] FoldedLetters fold_arabic(char32_t cp) noexcept {
    switch (cp) {
        case U'\U00000627':  // alif
        case U'\U00000621':  // hamza
        case U'\U00000623':  // alif with hamza above
        case U'\U00000625':  // alif with hamza below
        case U'\U00000622':  // alif with madda above
        case U'\U00000671':  // alif wasla
            return one(0);
        case U'\U00000628': return one(1);   // baa
        case U'\U0000062A': return one(2);   // taa
        case U'\U0000062B': return one(3);   // thaa
        case U'\U0000062C': return one(4);   // jiim
        case U'\U0000062D': return one(5);   // Haa
        case U'\U0000062E': return one(6);   // khaa
        case U'\U0000062F': return one(7);   // daal
        case U'\U00000630': return one(8);   // dhaal
        case U'\U00000631': return one(9);   // raa
        case U'\U00000632': return one(10);  // zaay
        case U'\U00000633': return one(11);  // siin
        case U'\U00000634': return one(12);  // shiin
        case U'\U00000635': return one(13);  // Saad
        case U'\U00000636': return one(14);  // Daad
        case U'\U00000637': return one(15);  // Taa
        case U'\U00000638': return one(16);  // Zaa
        case U'\U00000639': return one(17);  // ain
        case U'\U0000063A': return one(18);  // ghain
        case U'\U00000641': return one(19);  // faa
        case U'\U00000642': return one(20);  // qaaf
        case U'\U00000643': return one(21);  // kaaf
        case U'\U00000644': return one(22);  // laam
        case U'\U00000645': return one(23);  // miim
        case U'\U00000646': return one(24);  // nuun
        case U'\U00000647': return one(25);  // haa
        case U'\U00000648':  // waaw
        case U'\U00000624':  // waaw with hamza
            return one(26);
        case U'\U0000064A':  // yaa
        case U'\U00000626':  // yaa with hamza
        case U'\U00000649':  // alif maksura
            return one(27);
        case U'\U00000629':  // taa marbuta
            return one(28);
        default:
            return none();
    }
}

// Hebrew: 22 slots, alef..tav. Final letter forms fold to their base letter
// (see hebrew_alphabet()'s doc comment) - kaf/mem/nun/pe/tsadi sofit are
// purely positional variants, not distinct letters.
[[nodiscard]] FoldedLetters fold_hebrew(char32_t cp) noexcept {
    switch (cp) {
        case U'\U000005D0': return one(0);   // alef
        case U'\U000005D1': return one(1);   // bet
        case U'\U000005D2': return one(2);   // gimel
        case U'\U000005D3': return one(3);   // dalet
        case U'\U000005D4': return one(4);   // he
        case U'\U000005D5': return one(5);   // vav
        case U'\U000005D6': return one(6);   // zayin
        case U'\U000005D7': return one(7);   // het
        case U'\U000005D8': return one(8);   // tet
        case U'\U000005D9': return one(9);   // yod
        case U'\U000005DB':  // kaf
        case U'\U000005DA':  // final kaf (kaf sofit)
            return one(10);
        case U'\U000005DC': return one(11);  // lamed
        case U'\U000005DE':  // mem
        case U'\U000005DD':  // final mem (mem sofit)
            return one(12);
        case U'\U000005E0':  // nun
        case U'\U000005DF':  // final nun (nun sofit)
            return one(13);
        case U'\U000005E1': return one(14);  // samekh
        case U'\U000005E2': return one(15);  // ayin
        case U'\U000005E4':  // pe
        case U'\U000005E3':  // final pe (fe sofit)
            return one(16);
        case U'\U000005E6':  // tsadi
        case U'\U000005E5':  // final tsadi (tsadi sofit)
            return one(17);
        case U'\U000005E7': return one(18);  // qof
        case U'\U000005E8': return one(19);  // resh
        case U'\U000005E9': return one(20);  // shin
        case U'\U000005EA': return one(21);  // tav
        default:
            return none();
    }
}

// Yiddish: same 22-letter Hebrew-script base (delegates to fold_hebrew for
// everything it doesn't handle itself), plus three digraph letters Hebrew
// doesn't use, each expanded to its two-letter decomposition via the same
// ligature (count==2) mechanism French's oe/ae and German's Eszett already
// use - see yiddish_alphabet()'s doc comment. Yiddish's vowel-marking
// diacritic combinations (pasekh-alef, komets-alef, ...) need no case here
// at all: decode_utf8()'s combining-mark skip drops the diacritic before it
// ever reaches this function, leaving just the base alef.
[[nodiscard]] FoldedLetters fold_yiddish(char32_t cp) noexcept {
    switch (cp) {
        case U'\U000005F0':  // tsvey vovn ligature -> vav, vav
            return two(5, 5);
        case U'\U000005F1':  // vov yod ligature -> vav, yod
            return two(5, 9);
        case U'\U000005F2':  // tsvey yudn ligature -> yod, yod
            return two(9, 9);
        case U'\U0000FB4F':  // alef-lamed ligature -> alef, lamed
            return two(0, 11);
        default:
            return fold_hebrew(cp);
    }
}

}  // namespace

Alphabet english_alphabet() noexcept {
    Alphabet alphabet{
        AlphabetId::english, 26,
        +[](char32_t cp) noexcept -> FoldedLetters {
            if (cp > 0x7F) {
                return none();
            }
            const int index = letter_index(static_cast<char>(cp));
            return index < 0 ? none() : one(index);
        }};
    alphabet.display_letters = ascii_display_letters();
    alphabet.rack_size = 9;
    // Countdown draws its letter tiles from a Scrabble-weighted pool (the
    // show doesn't publish its own exact counts, but describes the
    // weighting as following Scrabble's); Y is treated as a consonant,
    // matching the show.
    apply_weights(alphabet.letter_weights, alphabet.is_vowel, {
        {0, 9, true},   // a
        {1, 2, false},  // b
        {2, 2, false},  // c
        {3, 4, false},  // d
        {4, 12, true},  // e
        {5, 2, false},  // f
        {6, 3, false},  // g
        {7, 2, false},  // h
        {8, 9, true},   // i
        {9, 1, false},  // j
        {10, 1, false}, // k
        {11, 4, false}, // l
        {12, 2, false}, // m
        {13, 6, false}, // n
        {14, 8, true},  // o
        {15, 2, false}, // p
        {16, 1, false}, // q
        {17, 6, false}, // r
        {18, 4, false}, // s
        {19, 6, false}, // t
        {20, 4, true},  // u
        {21, 2, false}, // v
        {22, 2, false}, // w
        {23, 1, false}, // x
        {24, 2, false}, // y
        {25, 1, false}, // z
    });
    // The only three legal vowel/consonant splits: at least 3 vowels, at
    // least 4 consonants, nine letters total.
    alphabet.vowel_consonant_splits = {{3, 6}, {4, 5}, {5, 4}};
    return alphabet;
}

Alphabet french_alphabet() noexcept {
    Alphabet alphabet{AlphabetId::french, 26,
                      +[](char32_t cp) noexcept { return fold_french(cp); }};
    alphabet.display_letters = ascii_display_letters();
    alphabet.rack_size = 10;  // Des chiffres et des lettres draws 10, not 9.
    // Sourced from the published French Scrabble tile distribution
    // (fr.wikipedia.org/wiki/Scrabble, en.wikipedia.org/wiki/Scrabble_letter_distributions).
    apply_weights(alphabet.letter_weights, alphabet.is_vowel, {
        {0, 9, true},    // a
        {1, 2, false},   // b
        {2, 2, false},   // c
        {3, 3, false},   // d
        {4, 15, true},   // e
        {5, 2, false},   // f
        {6, 2, false},   // g
        {7, 2, false},   // h
        {8, 8, true},    // i
        {9, 1, false},   // j
        {10, 1, false},  // k
        {11, 5, false},  // l
        {12, 3, false},  // m
        {13, 6, false},  // n
        {14, 6, true},   // o
        {15, 2, false},  // p
        {16, 1, false},  // q
        {17, 6, false},  // r
        {18, 6, false},  // s
        {19, 6, false},  // t
        {20, 6, true},   // u
        {21, 2, false},  // v
        {22, 1, false},  // w
        {23, 1, false},  // x
        {24, 1, false},  // y
        {25, 1, false},  // z
    });
    alphabet.vowel_consonant_splits = {{4, 6}, {5, 5}, {6, 4}};
    return alphabet;
}

Alphabet german_alphabet() noexcept {
    Alphabet alphabet{AlphabetId::german, 29,
                      +[](char32_t cp) noexcept { return fold_german(cp); }};
    alphabet.display_letters = ascii_display_letters();
    alphabet.display_letters.insert(alphabet.display_letters.end(),
                                     {"ä", "ö", "ü"});  // ä, ö, ü
    alphabet.rack_size = 9;  // No round-size difference found for Zahlen und Buchstaben.
    // Sourced from the published German Scrabble tile distribution
    // (en.wikipedia.org/wiki/Scrabble_letter_distributions). Ä/Ö/Ü are
    // vowels like their unmarked counterparts; Y is a rare, foreign-word
    // consonant as in English.
    apply_weights(alphabet.letter_weights, alphabet.is_vowel, {
        {0, 5, true},    // a
        {1, 2, false},   // b
        {2, 2, false},   // c
        {3, 4, false},   // d
        {4, 15, true},   // e
        {5, 2, false},   // f
        {6, 3, false},   // g
        {7, 4, false},   // h
        {8, 6, true},    // i
        {9, 1, false},   // j
        {10, 2, false},  // k
        {11, 3, false},  // l
        {12, 4, false},  // m
        {13, 9, false},  // n
        {14, 3, true},   // o
        {15, 1, false},  // p
        {16, 1, false},  // q
        {17, 6, false},  // r
        {18, 7, false},  // s
        {19, 6, false},  // t
        {20, 6, true},   // u
        {21, 1, false},  // v
        {22, 1, false},  // w
        {23, 1, false},  // x
        {24, 1, false},  // y
        {25, 1, false},  // z
        {26, 1, true},   // ä
        {27, 1, true},   // ö
        {28, 1, true},   // ü
    });
    alphabet.vowel_consonant_splits = {{3, 6}, {4, 5}, {5, 4}};
    return alphabet;
}

Alphabet spanish_alphabet() noexcept {
    Alphabet alphabet{AlphabetId::spanish, 27,
                      +[](char32_t cp) noexcept { return fold_spanish(cp); }};
    alphabet.display_letters = ascii_display_letters();
    alphabet.display_letters.push_back("ñ");  // ñ
    alphabet.rack_size = 9;  // Cifras y letras plays 9, same as English.
    // Approximate Spanish letter-frequency-weighted distribution (the show
    // doesn't publish its own counts either, same caveat as English above).
    // K/W are genuinely rare in Spanish (loanwords only) but not absent, so
    // they get a token weight rather than zero.
    apply_weights(alphabet.letter_weights, alphabet.is_vowel, {
        {0, 12, true},   // a
        {1, 2, false},   // b
        {2, 4, false},   // c
        {3, 5, false},   // d
        {4, 12, true},   // e
        {5, 1, false},   // f
        {6, 2, false},   // g
        {7, 2, false},   // h
        {8, 6, true},    // i
        {9, 1, false},   // j
        {10, 1, false},  // k
        {11, 4, false},  // l
        {12, 2, false},  // m
        {13, 5, false},  // n
        {14, 9, true},   // o
        {15, 2, false},  // p
        {16, 1, false},  // q
        {17, 5, false},  // r
        {18, 6, false},  // s
        {19, 4, false},  // t
        {20, 5, true},   // u
        {21, 1, false},  // v
        {22, 1, false},  // w
        {23, 1, false},  // x
        {24, 1, false},  // y
        {25, 1, false},  // z
        {26, 1, false},  // ñ
    });
    alphabet.vowel_consonant_splits = {{3, 6}, {4, 5}, {5, 4}};
    return alphabet;
}

Alphabet arabic_alphabet() noexcept {
    Alphabet alphabet{AlphabetId::arabic, 29,
                      +[](char32_t cp) noexcept { return fold_arabic(cp); }};
    // alif, baa, taa, thaa, jiim, Haa, khaa, daal, dhaal, raa, zaay, siin,
    // shiin, Saad, Daad, Taa, Zaa, ain, ghain, faa, qaaf, kaaf, laam, miim,
    // nuun, haa, waaw, yaa, taa marbuta.
    //
    // Written as raw UTF-8 byte escapes (\xNN\xNN), not \U0000XXXX universal
    // character names: this translation unit isn't built with /utf-8 (only
    // the app target is), so a \U-escape inside a plain narrow string
    // literal gets run through MSVC's default execution charset (Windows
    // code page 1252), which can't represent these codepoints at all and
    // turns it into a hard error. A \x byte escape is emitted verbatim, no
    // charset conversion involved, so it's encoding-agnostic like the
    // char32_t \U-escapes elsewhere in this file already are for a
    // different reason.
    alphabet.display_letters = {
        "\xD8\xA7", "\xD8\xA8", "\xD8\xAA", "\xD8\xAB", "\xD8\xAC",
        "\xD8\xAD", "\xD8\xAE", "\xD8\xAF", "\xD8\xB0", "\xD8\xB1",
        "\xD8\xB2", "\xD8\xB3", "\xD8\xB4", "\xD8\xB5", "\xD8\xB6",
        "\xD8\xB7", "\xD8\xB8", "\xD8\xB9", "\xD8\xBA", "\xD9\x81",
        "\xD9\x82", "\xD9\x83", "\xD9\x84", "\xD9\x85", "\xD9\x86",
        "\xD9\x87", "\xD9\x88", "\xD9\x8A", "\xD8\xA9",
    };
    alphabet.rack_size = 9;  // No reference game exists to justify anything else.
    // Approximate Arabic letter-frequency-weighted distribution - no
    // official adaptation exists to source exact counts from (see
    // arabic_alphabet()'s doc comment), so this is a good-faith estimate
    // from general letter-frequency knowledge, not a certified source.
    // Abjad script: no vowel_consonant_splits, so `vowel` is irrelevant here
    // and left false throughout.
    apply_weights(alphabet.letter_weights, alphabet.is_vowel, {
        {0, 15, false},   // alif
        {1, 7, false},    // baa
        {2, 7, false},    // taa
        {3, 2, false},    // thaa
        {4, 4, false},    // jiim
        {5, 4, false},    // Haa
        {6, 3, false},    // khaa
        {7, 5, false},    // daal
        {8, 2, false},    // dhaal
        {9, 7, false},    // raa
        {10, 2, false},   // zaay
        {11, 5, false},   // siin
        {12, 3, false},   // shiin
        {13, 3, false},   // Saad
        {14, 2, false},   // Daad
        {15, 3, false},   // Taa
        {16, 1, false},   // Zaa
        {17, 6, false},   // ain
        {18, 2, false},   // ghain
        {19, 5, false},   // faa
        {20, 5, false},   // qaaf
        {21, 5, false},   // kaaf
        {22, 12, false},  // laam
        {23, 9, false},   // miim
        {24, 9, false},   // nuun
        {25, 4, false},   // haa
        {26, 8, false},   // waaw
        {27, 10, false},  // yaa
        {28, 6, false},   // taa marbuta (very common feminine suffix)
    });
    // Empty on purpose - abjad script, no written short vowels, so
    // randomRack() falls back to a flat weighted draw (see
    // Alphabet::vowel_consonant_splits's doc comment).
    alphabet.vowel_consonant_splits = {};
    return alphabet;
}

Alphabet hebrew_alphabet() noexcept {
    Alphabet alphabet{AlphabetId::hebrew, 22,
                      +[](char32_t cp) noexcept { return fold_hebrew(cp); }};
    // alef, bet, gimel, dalet, he, vav, zayin, het, tet, yod, kaf, lamed,
    // mem, nun, samekh, ayin, pe, tsadi, qof, resh, shin, tav.
    alphabet.display_letters = {
        "\xD7\x90", "\xD7\x91", "\xD7\x92", "\xD7\x93", "\xD7\x94",
        "\xD7\x95", "\xD7\x96", "\xD7\x97", "\xD7\x98", "\xD7\x99",
        "\xD7\x9B", "\xD7\x9C", "\xD7\x9E", "\xD7\xA0", "\xD7\xA1",
        "\xD7\xA2", "\xD7\xA4", "\xD7\xA6", "\xD7\xA7", "\xD7\xA8",
        "\xD7\xA9", "\xD7\xAA",
    };
    alphabet.rack_size = 9;  // No reference game exists to justify anything else.
    // Approximate Hebrew letter-frequency-weighted distribution - same
    // good-faith-estimate caveat as Arabic above.
    apply_weights(alphabet.letter_weights, alphabet.is_vowel, {
        {0, 10, false},  // alef
        {1, 6, false},   // bet
        {2, 3, false},   // gimel
        {3, 4, false},   // dalet
        {4, 10, false},  // he
        {5, 12, false},  // vav
        {6, 3, false},   // zayin
        {7, 4, false},   // het
        {8, 3, false},   // tet
        {9, 12, false},  // yod
        {10, 5, false},  // kaf
        {11, 9, false},  // lamed
        {12, 9, false},  // mem
        {13, 8, false},  // nun
        {14, 3, false},  // samekh
        {15, 5, false},  // ayin
        {16, 5, false},  // pe
        {17, 3, false},  // tsadi
        {18, 4, false},  // qof
        {19, 8, false},  // resh
        {20, 6, false},  // shin
        {21, 6, false},  // tav
    });
    alphabet.vowel_consonant_splits = {};  // abjad script - see arabic_alphabet()
    return alphabet;
}

Alphabet yiddish_alphabet() noexcept {
    Alphabet alphabet{AlphabetId::yiddish, 22,
                      +[](char32_t cp) noexcept { return fold_yiddish(cp); }};
    // Same 22-letter Hebrew-script base and display glyphs as
    // hebrew_alphabet() - Yiddish's three extra digraph letters decompose
    // to two of these existing slots rather than needing dedicated ones
    // (see fold_yiddish()).
    alphabet.display_letters = {
        "\xD7\x90", "\xD7\x91", "\xD7\x92", "\xD7\x93", "\xD7\x94",
        "\xD7\x95", "\xD7\x96", "\xD7\x97", "\xD7\x98", "\xD7\x99",
        "\xD7\x9B", "\xD7\x9C", "\xD7\x9E", "\xD7\xA0", "\xD7\xA1",
        "\xD7\xA2", "\xD7\xA4", "\xD7\xA6", "\xD7\xA7", "\xD7\xA8",
        "\xD7\xA9", "\xD7\xAA",
    };
    alphabet.rack_size = 9;  // No reference game exists to justify anything else.
    // Reuses Hebrew's weight distribution as a starting approximation -
    // Yiddish is written in the same script, but as a Germanic (not
    // Semitic) language its real-world letter frequency genuinely differs;
    // this is a placeholder, not a claim they match.
    apply_weights(alphabet.letter_weights, alphabet.is_vowel, {
        {0, 10, false}, {1, 6, false}, {2, 3, false}, {3, 4, false},
        {4, 10, false}, {5, 12, false}, {6, 3, false}, {7, 4, false},
        {8, 3, false}, {9, 12, false}, {10, 5, false}, {11, 9, false},
        {12, 9, false}, {13, 8, false}, {14, 3, false}, {15, 5, false},
        {16, 5, false}, {17, 3, false}, {18, 4, false}, {19, 8, false},
        {20, 6, false}, {21, 6, false},
    });
    alphabet.vowel_consonant_splits = {};  // abjad-derived script - see arabic_alphabet()
    return alphabet;
}

std::vector<char32_t> decode_utf8(std::string_view text) {
    std::vector<char32_t> result;
    result.reserve(text.size());

    std::size_t i = 0;
    while (i < text.size()) {
        const auto byte_at = [&](std::size_t offset) noexcept {
            return static_cast<unsigned char>(text[i + offset]);
        };
        const unsigned char lead = byte_at(0);

        char32_t cp = 0;
        std::size_t len = 0;
        if ((lead & 0x80) == 0x00) {
            cp = lead;
            len = 1;
        } else if ((lead & 0xE0) == 0xC0) {
            cp = lead & 0x1F;
            len = 2;
        } else if ((lead & 0xF0) == 0xE0) {
            cp = lead & 0x0F;
            len = 3;
        } else if ((lead & 0xF8) == 0xF0) {
            cp = lead & 0x07;
            len = 4;
        } else {
            ++i;  // Not a valid UTF-8 leading byte - skip it and resync.
            continue;
        }

        if (i + len > text.size()) {
            ++i;  // Truncated sequence at the end of the input - skip the leading byte.
            continue;
        }

        bool valid = true;
        for (std::size_t k = 1; k < len; ++k) {
            const unsigned char continuation = byte_at(k);
            if ((continuation & 0xC0) != 0x80) {
                valid = false;
                break;
            }
            cp = (cp << 6) | (continuation & 0x3F);
        }
        if (!valid) {
            ++i;
            continue;
        }

        if (!is_combining_mark(cp)) {
            result.push_back(cp);
        }
        i += len;
    }

    return result;
}

Frequencies frequencies_of(std::string_view utf8_text, const Alphabet& alphabet) {
    Frequencies counts{};
    for (const char32_t cp : decode_utf8(utf8_text)) {
        const FoldedLetters folded = alphabet.fold(cp);
        for (int i = 0; i < folded.count; ++i) {
            ++counts[static_cast<std::size_t>(folded.indices[static_cast<std::size_t>(i)])];
        }
    }
    return counts;
}

}  // namespace countdown::letters
