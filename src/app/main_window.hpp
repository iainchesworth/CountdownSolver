#pragma once

#include <countdown/letters/dictionary.hpp>

#include <QMainWindow>

#include <optional>

class QLineEdit;
class QListWidget;
class QPlainTextEdit;
class QSpinBox;

namespace countdown::app {

// The single top-level window: a tab for the numbers game and a tab for the
// letters game, each driven entirely by countdown::solver.
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

private slots:
    void solveNumbers();
    void solveLetters();
    void showAbout();

private:
    [[nodiscard]] QWidget* buildNumbersTab();
    [[nodiscard]] QWidget* buildLettersTab();
    void buildMenus();
    void loadDictionary();

    // Numbers tab widgets.
    QLineEdit* numbers_input_ = nullptr;
    QSpinBox* target_input_ = nullptr;
    QPlainTextEdit* numbers_output_ = nullptr;

    // Letters tab widgets.
    QLineEdit* letters_input_ = nullptr;
    QListWidget* letters_output_ = nullptr;

    std::optional<countdown::letters::Dictionary> dictionary_;
};

}  // namespace countdown::app
