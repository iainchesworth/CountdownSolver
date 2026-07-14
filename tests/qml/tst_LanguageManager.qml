import QtQuick
import QtTest

// Exercises the `languageManager` context property registered by
// tests/qml/tst_qml_setup.hpp (the same LanguageManager main.cpp installs
// for the real app - see src/app/language_manager.hpp/.cpp).
TestCase {
    id: testCase
    name: "LanguageManager"

    function test_availableLanguagesListsAllSevenOptions() {
        const languages = languageManager.availableLanguages()
        compare(languages.length, 7)
        compare(languages[0].code, "en")
        verify(languages.some(function (l) { return l.code === "ar" }))
        verify(languages.some(function (l) { return l.code === "he" }))
        verify(languages.some(function (l) { return l.code === "yi" }))
    }

    function test_setLanguageSwitchesRejectsAndResets() {
        compare(languageManager.currentLanguage, "en")

        verify(languageManager.setLanguage("fr"))
        compare(languageManager.currentLanguage, "fr")

        // Re-selecting the already-active language is a no-op success, not
        // a translator reload.
        verify(languageManager.setLanguage("fr"))
        compare(languageManager.currentLanguage, "fr")

        verify(!languageManager.setLanguage("not-a-real-language"))
        compare(languageManager.currentLanguage, "fr")

        // Leaves global state clean for whichever test file runs next.
        verify(languageManager.setLanguage("en"))
        compare(languageManager.currentLanguage, "en")
    }
}
