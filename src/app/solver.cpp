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

// Loads the word list bundled into the binary as a Qt resource (see
// src/app/resources/dictionary/words.txt and the qt_add_resources call in
// src/app/CMakeLists.txt), giving the app a complete dictionary with no user
// setup required.
[[nodiscard]] letters::Dictionary load_default_dictionary() {
    QFile file(QStringLiteral(":/dictionary/words.txt"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCCritical(lcDictionary) << "failed to open bundled dictionary resource"
                                  << file.fileName() << ":" << file.errorString();
        return letters::Dictionary::from_words({});
    }

    std::vector<std::string> words;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        words.push_back(stream.readLine().toStdString());
    }
    letters::Dictionary dictionary = letters::Dictionary::from_words(words);
    if (dictionary.empty()) {
        qCWarning(lcDictionary) << "bundled dictionary resource" << file.fileName()
                                 << "opened but yielded no usable words";
    }
    return dictionary;
}

// Countdown draws its letter tiles from a Scrabble-weighted pool (the show
// doesn't publish its own exact counts, but describes the weighting as
// following Scrabble's); Y is treated as a consonant, matching the show.
constexpr std::array<std::pair<char, int>, 5> kVowelCounts{{
    {'a', 9}, {'e', 12}, {'i', 9}, {'o', 8}, {'u', 4},
}};
constexpr std::array<std::pair<char, int>, 21> kConsonantCounts{{
    {'b', 2}, {'c', 2}, {'d', 4}, {'f', 2}, {'g', 3}, {'h', 2}, {'j', 1},
    {'k', 1}, {'l', 4}, {'m', 2}, {'n', 6}, {'p', 2}, {'q', 1}, {'r', 6},
    {'s', 4}, {'t', 6}, {'v', 2}, {'w', 2}, {'x', 1}, {'y', 2}, {'z', 1},
}};
// The only three legal vowel/consonant splits: at least 3 vowels, at least 4
// consonants, nine letters total.
constexpr std::array<std::pair<int, int>, 3> kVowelConsonantSplits{{
    {3, 6}, {4, 5}, {5, 4},
}};

template <std::size_t N>
[[nodiscard]] std::vector<char> expand_tiles(const std::array<std::pair<char, int>, N>& counts) {
    std::vector<char> tiles;
    for (const auto& [letter, count] : counts) {
        tiles.insert(tiles.end(), static_cast<std::size_t>(count), letter);
    }
    return tiles;
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
[[nodiscard]] Result<letters::Dictionary> load_full_dictionary() {
    const platform::PlatformInfo info = platform::current();
    if (info.config_dir.empty()) {
        return std::unexpected(SolveError::dictionary_not_found);
    }
    return letters::Dictionary::load_from_file(info.config_dir / "words.txt");
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

void Solver::loadDictionaries() {
    default_dictionary_ = load_default_dictionary();
    full_dictionary_ = load_full_dictionary();
    dictionaries_ready_ = true;

    // A handful of sampled words (not the full list - that would flood the
    // log) confirms at a glance that the intended dictionary loaded and its
    // contents look sane, without adding a dedicated word-dump API.
    qCDebug(lcDictionary) << "default dictionary loaded:" << default_dictionary_.size()
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
    std::string word = candidates[pick(rng_)];
    std::ranges::shuffle(word, rng_);
    return QString::fromStdString(word).toUpper();
}

QString Solver::randomRack() const {
    // Draws from the same weighted tile pools the real game uses, respecting
    // one of the three legal vowel/consonant splits - not every rack will
    // yield a rich set of words, same as the real show.
    std::uniform_int_distribution<std::size_t> pick_split(0, kVowelConsonantSplits.size() - 1);
    const auto [vowel_count, consonant_count] = kVowelConsonantSplits[pick_split(rng_)];

    std::vector<char> vowels = expand_tiles(kVowelCounts);
    std::vector<char> consonants = expand_tiles(kConsonantCounts);
    std::ranges::shuffle(vowels, rng_);
    std::ranges::shuffle(consonants, rng_);

    std::string rack;
    rack.append(vowels.begin(), vowels.begin() + vowel_count);
    rack.append(consonants.begin(), consonants.begin() + consonant_count);
    std::ranges::shuffle(rack, rng_);
    return QString::fromStdString(rack).toUpper();
}

QString Solver::randomConundrum() const {
    return shuffledWord(9);
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

bool Solver::fullDictionaryAvailable() const {
    return full_dictionary_.has_value();
}

QString Solver::fullDictionaryStatus() const {
    if (full_dictionary_) {
        return QStringLiteral("A custom dictionary is available.");
    }
    if (full_dictionary_.error() == SolveError::dictionary_empty) {
        return QStringLiteral(
            "A words.txt file was found in the config folder, but it contained no usable words.");
    }
    return QStringLiteral("Add a words.txt file to the config folder to enable a custom dictionary.");
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
