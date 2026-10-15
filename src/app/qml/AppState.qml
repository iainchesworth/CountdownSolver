pragma Singleton
import QtQuick

// Runtime solver preferences, shared between the Settings page and the
// Numbers/Letters pages. (Visual theme lives in Theme.qml.)
//
// minLen, maxResults and flagInexact survive a restart: each is seeded from
// `preferences` (read from QSettings by main() before any QML loads) and
// written back whenever it changes. The write-back guards both skip a
// redundant write on startup and tolerate a null `preferences`, same as
// Theme.qml's `mode`. useFullDictionary is session-only on purpose - see
// preferences.hpp.
QtObject {
    property int  minLen: preferences ? preferences.minLen : 4        // shortest letters-game word to show (3–5)
    property int  maxResults: preferences ? preferences.maxResults : 60   // cap on letters-game result list (20–150)
    property bool flagInexact: preferences ? preferences.flagInexact : true // highlight the closest numbers result
    property bool useFullDictionary: false // solve from the full list instead of the sample

    onMinLenChanged: { if (preferences && preferences.minLen !== minLen) preferences.minLen = minLen }
    onMaxResultsChanged: { if (preferences && preferences.maxResults !== maxResults) preferences.maxResults = maxResults }
    onFlagInexactChanged: { if (preferences && preferences.flagInexact !== flagInexact) preferences.flagInexact = flagInexact }
}
