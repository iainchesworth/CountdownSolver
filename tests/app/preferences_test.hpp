#pragma once

#include <QObject>
#include <QTemporaryDir>

// countdown::app::Preferences: the persisted theme/solver options. Each case
// runs against a private, emptied INI store (never the developer's registry
// or config folder) and builds a fresh Preferences to stand in for a restart.
class PreferencesTest : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();

    void defaults_onFirstRun();
    void settersPersistAcrossInstances();
    void settersRejectInvalidValues();
    void invalidStoredValuesFallBackToDefaults();
    void unchangedValuesEmitNothing();

private:
    QTemporaryDir store_dir_;
};
