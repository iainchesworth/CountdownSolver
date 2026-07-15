#include "solver.hpp"

#include "logging/logging.hpp"
#include "platform/platform.hpp"

#include <countdown/conundrum/conundrum_game.hpp>
#include <countdown/error.hpp>
#include <countdown/numbers/numbers_game.hpp>
#include <countdown/version.hpp>

#include <QChar>
#include <QElapsedTimer>
#include <QFile>
#include <QFutureWatcher>
#include <QIODevice>
#include <QStringList>
#include <QTextStream>
#include <QtConcurrent/QtConcurrentRun>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace countdown::app {
namespace {

[[nodiscard]] QString to_qstring(SolveError error) {
    const std::string_view text = to_string(error);
    return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
}

[[nodiscard]] QStringList to_qstringlist(const std::vector<std::string>& words) {
    QStringList list;
    list.reserve(static_cast<qsizetype>(words.size()));
    for (const std::string& word : words) {
        list << QString::fromStdString(word);
    }
    return list;
}

// Logs the wall-clock duration of the enclosing function to lcSolver at
// destruction, covering every return path uniformly - useful context if a
// user reports the GUI "hanging" on a given rack/target.
class ScopedSolveTimer {
public:
    explicit ScopedSolveTimer(const char* label) : label_(label) { timer_.start(); }
    ~ScopedSolveTimer() { qCDebug(lcSolver) << label_ << "took" << timer_.elapsed() << "ms"; }

    ScopedSolveTimer(const ScopedSolveTimer&) = delete;
    ScopedSolveTimer& operator=(const ScopedSolveTimer&) = delete;

private:
    const char* label_;
    QElapsedTimer timer_;
};

// Maps a LanguageManager language code to the Alphabet Solver's game logic
// should use. Kept here (app layer), not in the library - the library has
// no business knowing about ISO-ish language codes.
[[nodiscard]] const letters::Alphabet& alphabet_for_language(const QString& code) {
    static const letters::Alphabet english = letters::english_alphabet();
    static const letters::Alphabet french = letters::french_alphabet();
    static const letters::Alphabet german = letters::german_alphabet();
    static const letters::Alphabet spanish = letters::spanish_alphabet();
    static const letters::Alphabet arabic = letters::arabic_alphabet();
    static const letters::Alphabet hebrew = letters::hebrew_alphabet();
    static const letters::Alphabet yiddish = letters::yiddish_alphabet();
    if (code == QStringLiteral("fr")) return french;
    if (code == QStringLiteral("de")) return german;
    if (code == QStringLiteral("es")) return spanish;
    if (code == QStringLiteral("ar")) return arabic;
    if (code == QStringLiteral("he")) return hebrew;
    if (code == QStringLiteral("yi")) return yiddish;
    return english;
}

// Maps a language code to its bundled default-dictionary resource path,
// falling back to the English word list for any language without its own
// placeholder/real dictionary yet.
[[nodiscard]] QString dictionary_resource_for_language(const QString& code) {
    if (code == QStringLiteral("fr")) return QStringLiteral(":/dictionary/words_fr.txt");
    if (code == QStringLiteral("de")) return QStringLiteral(":/dictionary/words_de.txt");
    if (code == QStringLiteral("es")) return QStringLiteral(":/dictionary/words_es.txt");
    if (code == QStringLiteral("ar")) return QStringLiteral(":/dictionary/words_ar.txt");
    if (code == QStringLiteral("he")) return QStringLiteral(":/dictionary/words_he.txt");
    if (code == QStringLiteral("yi")) return QStringLiteral(":/dictionary/words_yi.txt");
    return QStringLiteral(":/dictionary/words_en.txt");
}

// Loads the word list bundled into the binary as a Qt resource (see
// src/app/resources/dictionary/ and the qt_add_resources call in
// src/app/CMakeLists.txt), giving the app a complete dictionary with no user
// setup required.
[[nodiscard]] letters::Dictionary load_default_dictionary(const QString& resource_path,
                                                           const letters::Alphabet& alphabet) {
    QFile file(resource_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCCritical(lcDictionary) << "failed to open bundled dictionary resource"
                                  << file.fileName() << ":" << file.errorString();
        return letters::Dictionary::from_words({}, alphabet);
    }

    std::vector<std::string> words;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        words.push_back(stream.readLine().toStdString());
    }
    letters::Dictionary dictionary = letters::Dictionary::from_words(words, alphabet);
    if (dictionary.empty()) {
        qCWarning(lcDictionary) << "bundled dictionary resource" << file.fileName()
                                 << "opened but yielded no usable words";
    }
    return dictionary;
}

[[nodiscard]] QString op_symbol(numbers::Op op) {
    switch (op) {
        case numbers::Op::add:      return QStringLiteral("+");
        case numbers::Op::subtract: return QString(QChar(0x2212));  // MINUS SIGN
        case numbers::Op::multiply: return QString(QChar(0x00D7));  // MULTIPLICATION SIGN
        case numbers::Op::divide:   return QString(QChar(0x00F7));  // DIVISION SIGN
    }
    return QStringLiteral("?");
}

// Unlike load_default_dictionary() above, failure here is an expected,
// common state (most users never place a custom words.txt), so the
// distinguishing SolveError is preserved rather than collapsed to
// std::nullopt: Solver::fullDictionaryStatus() surfaces it to the Settings UI,
// and it's logged here for anyone diagnosing headlessly.
[[nodiscard]] Result<letters::Dictionary> load_full_dictionary(const letters::Alphabet& alphabet) {
    const platform::PlatformInfo info = platform::current();
    if (info.config_dir.empty()) {
        return std::unexpected(SolveError::dictionary_not_found);
    }
    return letters::Dictionary::load_from_file(info.config_dir / "words.txt", alphabet);
}

}  // namespace

Solver::Solver(QObject* parent, DictionaryLoad load)
    : QObject(parent),
      // Cheap placeholders, immediately valid ("no words" / "not found") so
      // active_dictionary() never needs to special-case an unloaded state -
      // loadDictionaries() below replaces both with the real data, either
      // right away (kEager) or once main.cpp defers it past engine startup
      // (kDeferred).
      default_dictionary_(letters::Dictionary::from_words({})),
      full_dictionary_(std::unexpected(SolveError::dictionary_not_found)),
      rng_(std::random_device{}()) {
    if (load == DictionaryLoad::kEager) {
        loadDictionaries();
    }
}

void Solver::setLanguageCode(const QString& code) {
    if (code == language_code_) {
        return;
    }
    language_code_ = code;
    if (dictionaries_ready_) {
        loadDictionaries();
    }
    // Else: dictionaries haven't loaded yet - the deferred first
    // loadDictionaries() call (see main.cpp) will pick up language_code_.
}

void Solver::loadDictionaries() {
    const letters::Alphabet& alphabet = alphabet_for_language(language_code_);
    default_dictionary_ = load_default_dictionary(dictionary_resource_for_language(language_code_), alphabet);
    full_dictionary_ = load_full_dictionary(alphabet);
    dictionaries_ready_ = true;

    // A handful of sampled words (not the full list - that would flood the
    // log) confirms at a glance that the intended dictionary loaded and its
    // contents look sane, without adding a dedicated word-dump API.
    qCDebug(lcDictionary) << "default dictionary loaded for" << language_code_
                           << "(rack size" << alphabet.rack_size << "):"
                           << default_dictionary_.size()
                           << "words; sample:" << to_qstringlist(default_dictionary_.sample(500, 5));
    if (full_dictionary_) {
        qCDebug(lcDictionary) << "full dictionary loaded:" << full_dictionary_->size()
                               << "words; sample:" << to_qstringlist(full_dictionary_->sample(500, 5));
    } else {
        qCInfo(lcDictionary) << "full dictionary unavailable:" << to_qstring(full_dictionary_.error());
    }

    emit dictionariesReadyChanged();
}

const letters::Dictionary& Solver::active_dictionary() const {
    return (using_full_dictionary_ && full_dictionary_) ? *full_dictionary_ : default_dictionary_;
}

QVariantMap Solver::solveNumbers(const QVariantList& numbers, int target) const {
    const ScopedSolveTimer timer_guard("solveNumbers");

    std::vector<int> chosen;
    chosen.reserve(static_cast<std::size_t>(numbers.size()));
    for (const QVariant& value : numbers) {
        chosen.push_back(value.toInt());
    }

    QVariantMap result;
    const auto outcome = numbers::NumbersGame{}.with_target(target).with_numbers(chosen).solve();
    if (!outcome) {
        // NumbersGame::solve() always keeps a "closest so far" candidate once
        // validate() passes (even a single valid term is one), so a failure
        // here is always a validation problem (empty_input/target_out_of_range/
        // number_out_of_range) - worth logging unconditionally rather than
        // gating on which SolveError it is.
        qCWarning(lcSolver) << "solveNumbers rejected input:" << to_qstring(outcome.error());
        result["value"] = 0;
        result["diff"] = target;
        result["exact"] = false;
        result["steps"] = QStringList{};
        return result;
    }

    const numbers::Value value = outcome->solution.value();
    const numbers::Value diff = value > target ? value - target : target - value;

    QStringList steps;
    for (const numbers::Step& step : outcome->solution.steps()) {
        steps << QStringLiteral("%1 %2 %3 = %4")
                     .arg(static_cast<qlonglong>(step.lhs))
                     .arg(op_symbol(step.op))
                     .arg(static_cast<qlonglong>(step.rhs))
                     .arg(static_cast<qlonglong>(step.result));
    }

    result["value"] = static_cast<qlonglong>(value);
    result["diff"] = static_cast<qlonglong>(diff);
    result["exact"] = outcome->exact;
    result["steps"] = steps;
    return result;
}

void Solver::solveNumbersAsync(const QVariantList& numbers, int target) {
    // solveNumbers() touches only its arguments and NumbersGame's local
    // state - no dictionary access - so running it on QtConcurrent's thread
    // pool needs no synchronization with the rest of Solver.
    auto* watcher = new QFutureWatcher<QVariantMap>(this);
    connect(watcher, &QFutureWatcherBase::finished, this, [this, watcher]() {
        emit numbersSolved(watcher->result());
        watcher->deleteLater();
    });
    watcher->setFuture(QtConcurrent::run(
        [this, numbers, target]() { return solveNumbers(numbers, target); }));
}

QVariantMap Solver::solveLetters(const QString& rack, int minLen, int maxResults) const {
    const ScopedSolveTimer timer_guard("solveLetters");

    QVariantMap result;
    result["total"] = 0;
    result["shown"] = 0;
    result["maxLen"] = 0;
    result["longest"] = QStringList{};
    result["groups"] = QVariantList{};

    const std::string letters = rack.toLower().toStdString();
    const std::size_t min_length = minLen > 0 ? static_cast<std::size_t>(minLen) : 1;

    const auto matches = active_dictionary().find_matches(letters, min_length);
    if (!matches) {
        // Anything other than the routine "no rack produces a match" is
        // worth flagging - most notably dictionary_empty, an active
        // dictionary that failed to load properly rather than a legitimate
        // no-matches result.
        if (matches.error() != SolveError::no_solution) {
            qCWarning(lcSolver) << "solveLetters rejected input:" << to_qstring(matches.error());
        }
        return result;
    }
    const std::vector<std::string>& all = *matches;  // longest first, alpha within

    const std::size_t max_len = all.front().size();
    QStringList longest;
    for (const std::string& word : all) {
        if (word.size() != max_len) {
            break;
        }
        longest << QString::fromStdString(word);
    }

    const std::size_t cap = maxResults > 0 ? static_cast<std::size_t>(maxResults) : all.size();
    const std::size_t shown = std::min(cap, all.size());

    // True per-length totals across every match, independent of the
    // maxResults cap below - otherwise a group's word count would silently
    // reflect only however many happened to fit before the cap ran out.
    std::unordered_map<int, int> length_counts;
    for (const std::string& word : all) {
        ++length_counts[static_cast<int>(word.size())];
    }

    // Group the shown words by length (descending, already sorted that way).
    QVariantList groups;
    QStringList group_words;
    int group_len = -1;
    const auto flush = [&] {
        if (group_len != -1) {
            QVariantMap group;
            group["len"] = group_len;
            group["count"] = length_counts[group_len];
            group["words"] = group_words;
            groups.push_back(group);
        }
    };
    for (std::size_t i = 0; i < shown; ++i) {
        const int len = static_cast<int>(all[i].size());
        if (len != group_len) {
            flush();
            group_len = len;
            group_words.clear();
        }
        group_words << QString::fromStdString(all[i]);
    }
    flush();

    result["total"] = static_cast<int>(all.size());
    result["shown"] = static_cast<int>(shown);
    result["maxLen"] = static_cast<int>(max_len);
    result["longest"] = longest;
    result["groups"] = groups;
    return result;
}

QVariantMap Solver::solveConundrum(const QString& letters) const {
    const ScopedSolveTimer timer_guard("solveConundrum");

    QVariantMap result;
    result["found"] = false;
    result["answers"] = QStringList{};

    const std::string rack = letters.toLower().toStdString();
    const auto anagrams = conundrum::ConundrumGame{active_dictionary()}.with_letters(rack).solve();
    if (!anagrams) {
        // See the equivalent check in solveLetters() above.
        if (anagrams.error() != SolveError::no_solution) {
            qCWarning(lcSolver) << "solveConundrum rejected input:" << to_qstring(anagrams.error());
        }
        return result;
    }

    QStringList answers;
    for (const std::string& word : *anagrams) {
        answers << QString::fromStdString(word);
    }
    result["found"] = !answers.isEmpty();
    result["answers"] = answers;
    return result;
}

QString Solver::shuffledWord(std::size_t length) const {
    const std::vector<std::string> candidates = active_dictionary().words_of_length(length);
    if (candidates.empty()) {
        return {};
    }
    std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
    // Shuffles decoded codepoints, not raw bytes: a byte-level shuffle would
    // silently corrupt any multi-byte UTF-8 letter (é, ñ, ä, ...) by
    // splitting its continuation bytes across the string. Only matters once
    // a non-English dictionary can supply this word, but it's just as wrong
    // for English in principle - fixed generally rather than special-cased.
    std::vector<char32_t> codepoints = letters::decode_utf8(candidates[pick(rng_)]);
    std::ranges::shuffle(codepoints, rng_);
    return QString::fromUcs4(codepoints.data(), static_cast<qsizetype>(codepoints.size())).toUpper();
}

QString Solver::randomRack() const {
    const letters::Alphabet& alphabet = active_dictionary().alphabet();

    std::vector<int> rack_slots;
    if (!alphabet.vowel_consonant_splits.empty()) {
        // Draws from the alphabet's weighted tile pools, respecting one of
        // its legal vowel/consonant splits - not every rack will yield a
        // rich set of words, same as the real show.
        std::uniform_int_distribution<std::size_t> pick_split(
            0, alphabet.vowel_consonant_splits.size() - 1);
        const auto [vowel_count, consonant_count] = alphabet.vowel_consonant_splits[pick_split(rng_)];

        std::vector<int> vowel_pool;
        std::vector<int> consonant_pool;
        for (std::size_t i = 0; i < alphabet.size; ++i) {
            std::vector<int>& pool = alphabet.is_vowel[i] ? vowel_pool : consonant_pool;
            pool.insert(pool.end(), static_cast<std::size_t>(alphabet.letter_weights[i]),
                        static_cast<int>(i));
        }
        std::ranges::shuffle(vowel_pool, rng_);
        std::ranges::shuffle(consonant_pool, rng_);

        rack_slots.insert(rack_slots.end(), vowel_pool.begin(),
                     vowel_pool.begin() + static_cast<std::ptrdiff_t>(vowel_count));
        rack_slots.insert(rack_slots.end(), consonant_pool.begin(),
                     consonant_pool.begin() + static_cast<std::ptrdiff_t>(consonant_count));
    } else {
        // No split defined (abjad scripts): flat weighted draw of rack_size
        // letter slots with no vowel/consonant constraint.
        std::vector<int> pool;
        for (std::size_t i = 0; i < alphabet.size; ++i) {
            pool.insert(pool.end(), static_cast<std::size_t>(alphabet.letter_weights[i]),
                        static_cast<int>(i));
        }
        std::ranges::shuffle(pool, rng_);
        const std::size_t take = std::min(alphabet.rack_size, pool.size());
        rack_slots.assign(pool.begin(), pool.begin() + static_cast<std::ptrdiff_t>(take));
    }
    std::ranges::shuffle(rack_slots, rng_);

    QString rack;
    for (const int slot : rack_slots) {
        rack += QString::fromStdString(alphabet.display_letters[static_cast<std::size_t>(slot)]);
    }
    return rack.toUpper();
}

QString Solver::randomConundrum() const {
    return shuffledWord(active_dictionary().alphabet().rack_size);
}

int Solver::rackSize() const {
    return static_cast<int>(active_dictionary().alphabet().rack_size);
}

QString Solver::versionDetails() const {
    return QString::fromStdString(countdown::version_details());
}

QString Solver::shortVersion() const {
    return QString::fromUtf8(countdown::version_string.data(),
                              static_cast<qsizetype>(countdown::version_string.size()));
}

bool Solver::isDirty() const {
    return countdown::git_dirty;
}

QString Solver::qtVersion() const {
    return QString::fromUtf8(qVersion());
}

bool Solver::fullDictionaryAvailable() const {
    return full_dictionary_.has_value();
}

QString Solver::fullDictionaryStatus() const {
    if (full_dictionary_) {
        return tr("A custom dictionary is available.");
    }
    if (full_dictionary_.error() == SolveError::dictionary_empty) {
        return tr("A words.txt file was found in the config folder, but it contained no usable words.");
    }
    return tr("Add a words.txt file to the config folder to enable a custom dictionary.");
}

bool Solver::usingFullDictionary() const {
    return using_full_dictionary_;
}

bool Solver::setUseFullDictionary(bool full) {
    if (full && !full_dictionary_) {
        return false;
    }
    using_full_dictionary_ = full;
    return true;
}

int Solver::dictionaryWordCount() const {
    return static_cast<int>(active_dictionary().size());
}

}  // namespace countdown::app
