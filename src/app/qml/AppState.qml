pragma Singleton
import QtQuick

// Runtime solver preferences, shared between the Settings page and the
// Numbers/Letters pages. (Visual theme lives in Theme.qml.)
QtObject {
    property int  minLen: 4        // shortest letters-game word to show (3–5)
    property int  maxResults: 60   // cap on letters-game result list (20–150)
    property bool flagInexact: true // highlight the closest numbers result
    property bool useFullDictionary: false // solve from the full list instead of the sample
}
