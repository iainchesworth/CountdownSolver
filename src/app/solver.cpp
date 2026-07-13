#include "solver.hpp"

#include "platform/platform.hpp"

#include <countdown/numbers/numbers_game.hpp>
#include <countdown/version.hpp>

#include <QChar>
#include <QFile>
#include <QIODevice>
#include <QStringList>
#include <QTextStream>

#include <algorithm>
#include <cstdlib>
#include <string>
#include <vector>

namespace countdown::app {
namespace {

// Loads the word list bundled into the binary as a Qt resource (see
// src/app/resources/dictionary/words.txt and the qt_add_resources call in
// src/app/CMakeLists.txt), giving the app a complete dictionary with no user
// setup required.
[[nodiscard]] letters::Dictionary load_default_dictionary() {
    QFile file(QStringLiteral(":/dictionary/words.txt"));
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    std::vector<std::string> words;
    QTextStream stream(&file);
    while (!stream.atEnd()) {
        words.push_back(stream.readLine().toStdString());
    }
    return letters::Dictionary::from_words(words);
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

[[nodiscard]] std::optional<letters::Dictionary> load_full_dictionary() {
    const platform::PlatformInfo info = platform::current();
    if (info.config_dir.empty()) {
        return std::nullopt;
    }
    if (auto loaded = letters::Dictionary::load_from_file(info.config_dir / "words.txt")) {
        return *std::move(loaded);
    }
    return std::nullopt;
}

}  // namespace

Solver::Solver(QObject* parent)
    : QObject(parent),
      default_dictionary_(load_default_dictionary()),
      full_dictionary_(load_full_dictionary()),
      rng_(std::random_device{}()) {}

const letters::Dictionary& Solver::active_dictionary() const {
    return (using_full_dictionary_ && full_dictionary_) ? *full_dictionary_ : default_dictionary_;
}

QVariantMap Solver::solveNumbers(const QVariantList& numbers, int target) const {
    std::vector<int> chosen;
    chosen.reserve(static_cast<std::size_t>(numbers.size()));
    for (const QVariant& value : numbers) {
        chosen.push_back(value.toInt());
    }

    QVariantMap result;
    const auto outcome = numbers::NumbersGame{}.with_target(target).with_numbers(chosen).solve();
    if (!outcome) {
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

QVariantMap Solver::solveLetters(const QString& rack, int minLen, int maxResults) const {
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

    // Group the shown words by length (descending, already sorted that way).
    QVariantList groups;
    QStringList group_words;
    int group_len = -1;
    const auto flush = [&] {
        if (group_len != -1) {
            QVariantMap group;
            group["len"] = group_len;
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
    QVariantMap result;
    result["found"] = false;
    result["answers"] = QStringList{};

    const std::string rack = letters.toLower().toStdString();
    // A full anagram is a match whose length equals the rack length.
    const auto anagrams = active_dictionary().find_matches(rack, rack.size());
    if (!anagrams) {
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
    // Prefer a shuffled real word so the rack yields rich results.
    for (const std::size_t length : {std::size_t{9}, std::size_t{8}}) {
        if (QString rack = shuffledWord(length); !rack.isEmpty()) {
            return rack;
        }
    }
    return {};
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

bool Solver::fullDictionaryAvailable() const {
    return full_dictionary_.has_value();
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
