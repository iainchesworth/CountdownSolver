#include "main_window.hpp"

#include "platform/platform.hpp"

#include <countdown/numbers/numbers_game.hpp>
#include <countdown/letters/letters_game.hpp>
#include <countdown/version.hpp>

#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRegularExpression>
#include <QSpinBox>
#include <QString>
#include <QStringList>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QWidget>

#include <string>
#include <vector>

namespace countdown::app {
namespace {

[[nodiscard]] QString toQString(std::string_view text) {
    return QString::fromUtf8(text.data(), static_cast<qsizetype>(text.size()));
}

// A minimal built-in word list so the letters game works before the user
// supplies a full dictionary at <config_dir>/words.txt.
const std::vector<std::string> kFallbackWords = {
    "countdown", "solver", "numbers", "letters", "conundrum",
    "vowel",     "consonant", "arithmetic", "integer", "operation",
    "triangle",  "rectangle", "notice",    "orient",  "creation",
    "reaction",  "cratering", "teaching",  "cheating",
};

}  // namespace

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(tr("Countdown Solver"));

    auto* tabs = new QTabWidget(this);
    tabs->addTab(buildNumbersTab(), tr("Numbers"));
    tabs->addTab(buildLettersTab(), tr("Letters"));
    setCentralWidget(tabs);

    buildMenus();
    loadDictionary();
    resize(560, 420);
}

void MainWindow::buildMenus() {
    auto* help_menu = menuBar()->addMenu(tr("&Help"));
    help_menu->addAction(tr("&About"), this, &MainWindow::showAbout);
}

void MainWindow::showAbout() {
    QMessageBox::about(this, tr("About Countdown Solver"),
                       QString::fromStdString(countdown::version_details()));
}

QWidget* MainWindow::buildNumbersTab() {
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);

    auto* form = new QFormLayout();
    numbers_input_ = new QLineEdit(tab);
    numbers_input_->setPlaceholderText(tr("e.g. 75 25 3 6 2 1"));
    target_input_ = new QSpinBox(tab);
    target_input_->setRange(100, 999);
    target_input_->setValue(532);
    form->addRow(tr("Numbers:"), numbers_input_);
    form->addRow(tr("Target:"), target_input_);
    layout->addLayout(form);

    auto* solve_button = new QPushButton(tr("Solve"), tab);
    layout->addWidget(solve_button);

    numbers_output_ = new QPlainTextEdit(tab);
    numbers_output_->setReadOnly(true);
    layout->addWidget(numbers_output_);

    connect(solve_button, &QPushButton::clicked, this, &MainWindow::solveNumbers);
    return tab;
}

QWidget* MainWindow::buildLettersTab() {
    auto* tab = new QWidget(this);
    auto* layout = new QVBoxLayout(tab);

    auto* row = new QHBoxLayout();
    row->addWidget(new QLabel(tr("Letters:"), tab));
    letters_input_ = new QLineEdit(tab);
    letters_input_->setPlaceholderText(tr("e.g. rateciong"));
    row->addWidget(letters_input_);
    layout->addLayout(row);

    auto* solve_button = new QPushButton(tr("Solve"), tab);
    layout->addWidget(solve_button);

    letters_output_ = new QListWidget(tab);
    layout->addWidget(letters_output_);

    connect(solve_button, &QPushButton::clicked, this, &MainWindow::solveLetters);
    return tab;
}

void MainWindow::loadDictionary() {
    // Prefer a user-supplied dictionary at the platform config directory; fall
    // back to the small built-in list otherwise.
    const platform::PlatformInfo info = platform::current();
    if (!info.config_dir.empty()) {
        const auto path = info.config_dir / "words.txt";
        if (auto loaded = letters::Dictionary::load_from_file(path)) {
            dictionary_ = *std::move(loaded);
            return;
        }
    }
    dictionary_ = letters::Dictionary::from_words(kFallbackWords);
}

void MainWindow::solveNumbers() {
    const QStringList tokens =
        numbers_input_->text().split(QRegularExpression("[^0-9]+"), Qt::SkipEmptyParts);

    std::vector<int> numbers;
    numbers.reserve(static_cast<std::size_t>(tokens.size()));
    for (const QString& token : tokens) {
        numbers.push_back(token.toInt());
    }

    const auto outcome = numbers::NumbersGame{}
                             .with_target(target_input_->value())
                             .with_numbers(numbers)
                             .solve();

    if (!outcome) {
        numbers_output_->setPlainText(
            tr("No solution: %1").arg(toQString(to_string(outcome.error()))));
        return;
    }

    const auto& value = outcome->solution.value();
    QString header = outcome->exact
                         ? tr("Exact solution (%1):\n").arg(value)
                         : tr("Closest: %1\n").arg(value);
    numbers_output_->setPlainText(header + QString::fromStdString(outcome->solution.describe()));
}

void MainWindow::solveLetters() {
    letters_output_->clear();
    if (!dictionary_) {
        letters_output_->addItem(tr("No dictionary loaded."));
        return;
    }

    const auto game = letters::LettersGame{*dictionary_}
                          .with_letters(letters_input_->text().toStdString());
    const auto words = game.solve();

    if (!words) {
        letters_output_->addItem(tr("No word: %1").arg(toQString(to_string(words.error()))));
        return;
    }
    for (const std::string& word : *words) {
        letters_output_->addItem(QString::fromStdString(word));
    }
}

}  // namespace countdown::app
