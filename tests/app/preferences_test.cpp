#include "preferences_test.hpp"

#include "preferences.hpp"

#include <QCoreApplication>
#include <QSettings>
#include <QSignalSpy>
#include <QTest>

using countdown::app::Preferences;

void PreferencesTest::initTestCase() {
    QVERIFY(store_dir_.isValid());

    // Static configuration only (this binary has no QCoreApplication): every
    // default-constructed QSettings from here on - Preferences' own included -
    // reads and writes a private INI file under store_dir_.
    QCoreApplication::setOrganizationName(QStringLiteral("CountdownSolverTests"));
    QCoreApplication::setApplicationName(QStringLiteral("preferences_test"));
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QSettings::setPath(QSettings::IniFormat, QSettings::UserScope, store_dir_.path());
}

void PreferencesTest::cleanupTestCase() {
    // Put the process-wide QSettings configuration back, so the test classes
    // that run after this one see what they would have seen without it.
    QSettings::setDefaultFormat(QSettings::NativeFormat);
    QCoreApplication::setOrganizationName(QString());
    QCoreApplication::setApplicationName(QString());
}

void PreferencesTest::init() {
    QSettings settings;
    settings.clear();
    settings.sync();
}

void PreferencesTest::defaults_onFirstRun() {
    const Preferences preferences;

    QCOMPARE(preferences.theme(), QStringLiteral("system"));
    QCOMPARE(preferences.minLen(), 4);
    QCOMPARE(preferences.maxResults(), 60);
    QCOMPARE(preferences.flagInexact(), true);
}

void PreferencesTest::settersPersistAcrossInstances() {
    {
        Preferences preferences;
        preferences.setTheme(QStringLiteral("dark"));
        preferences.setMinLen(5);
        preferences.setMaxResults(120);
        preferences.setFlagInexact(false);
    }

    // The stored keys are part of the app's on-disk format ("group/name", like
    // language/code), so a rename needs a migration - pin them.
    const QSettings settings;
    QCOMPARE(settings.value(QStringLiteral("appearance/theme")).toString(), QStringLiteral("dark"));
    QCOMPARE(settings.value(QStringLiteral("solver/minLen")).toInt(), 5);
    QCOMPARE(settings.value(QStringLiteral("solver/maxResults")).toInt(), 120);
    QCOMPARE(settings.value(QStringLiteral("solver/flagInexact")).toBool(), false);

    const Preferences restarted;
    QCOMPARE(restarted.theme(), QStringLiteral("dark"));
    QCOMPARE(restarted.minLen(), 5);
    QCOMPARE(restarted.maxResults(), 120);
    QCOMPARE(restarted.flagInexact(), false);
}

void PreferencesTest::settersRejectInvalidValues() {
    Preferences preferences;
    preferences.setTheme(QStringLiteral("dark"));
    preferences.setMinLen(5);
    preferences.setMaxResults(100);

    preferences.setTheme(QStringLiteral("sepia"));
    preferences.setTheme(QString());
    preferences.setMinLen(2);
    preferences.setMinLen(6);
    preferences.setMaxResults(19);
    preferences.setMaxResults(151);

    QCOMPARE(preferences.theme(), QStringLiteral("dark"));
    QCOMPARE(preferences.minLen(), 5);
    QCOMPARE(preferences.maxResults(), 100);

    // Nothing invalid reached storage either.
    const Preferences restarted;
    QCOMPARE(restarted.theme(), QStringLiteral("dark"));
    QCOMPARE(restarted.minLen(), 5);
    QCOMPARE(restarted.maxResults(), 100);

    // The inclusive bounds themselves are valid.
    preferences.setMinLen(3);
    preferences.setMaxResults(20);
    QCOMPARE(preferences.minLen(), 3);
    QCOMPARE(preferences.maxResults(), 20);
    preferences.setMaxResults(150);
    QCOMPARE(preferences.maxResults(), 150);
}

void PreferencesTest::invalidStoredValuesFallBackToDefaults() {
    {
        QSettings settings;
        settings.setValue(QStringLiteral("appearance/theme"), QStringLiteral("neon"));
        settings.setValue(QStringLiteral("solver/minLen"), 99);
        settings.setValue(QStringLiteral("solver/maxResults"), QStringLiteral("lots"));
        settings.setValue(QStringLiteral("solver/flagInexact"), QStringLiteral("false"));
    }

    // A hand-edited or stale settings file must never wedge the UI: every
    // unusable value reads as "never saved".
    const Preferences preferences;
    QCOMPARE(preferences.theme(), QStringLiteral("system"));
    QCOMPARE(preferences.minLen(), 4);
    QCOMPARE(preferences.maxResults(), 60);
    // A usable (if oddly typed) value is honoured, not discarded.
    QCOMPARE(preferences.flagInexact(), false);
}

void PreferencesTest::unchangedValuesEmitNothing() {
    Preferences preferences;
    QSignalSpy theme(&preferences, &Preferences::themeChanged);
    QSignalSpy minLen(&preferences, &Preferences::minLenChanged);
    QSignalSpy maxResults(&preferences, &Preferences::maxResultsChanged);
    QSignalSpy flagInexact(&preferences, &Preferences::flagInexactChanged);

    preferences.setTheme(QStringLiteral("system"));
    preferences.setMinLen(4);
    preferences.setMaxResults(60);
    preferences.setFlagInexact(true);
    QCOMPARE(theme.count(), 0);
    QCOMPARE(minLen.count(), 0);
    QCOMPARE(maxResults.count(), 0);
    QCOMPARE(flagInexact.count(), 0);

    preferences.setTheme(QStringLiteral("light"));
    preferences.setMinLen(3);
    preferences.setMaxResults(70);
    preferences.setFlagInexact(false);
    QCOMPARE(theme.count(), 1);
    QCOMPARE(minLen.count(), 1);
    QCOMPARE(maxResults.count(), 1);
    QCOMPARE(flagInexact.count(), 1);
}
