import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// Settings. Writes straight into the Theme (visual) and AppState (solver) singletons.
Item {
    id: root

    // Popup/Dialog isn't a QQuickItem, so it never shows up in another
    // Item's `children` list - TestCase.findChild (a `.children` walk) can't
    // reach it. Expose it as a property alias instead, for
    // tests/qml/tst_SettingsPage.qml.
    property alias licensesDialog: licensesDialogInstance

    Flickable {
        anchors.fill: parent
        anchors.leftMargin: 24; anchors.rightMargin: 24; anchors.bottomMargin: 24
        contentHeight: col.implicitHeight
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        ColumnLayout {
            id: col
            width: parent.width
            spacing: 16

            // ---- Appearance ----
            Card {
                Layout.fillWidth: true
                implicitHeight: ap.implicitHeight + 40
                ColumnLayout {
                    id: ap
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 16
                    SectionLabel { text: qsTr("Appearance") }
                    Item {
                        Layout.fillWidth: true
                        implicitHeight: themeRow.implicitHeight
                        RowLayout {
                            id: themeRow
                            anchors.fill: parent
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Text { Layout.fillWidth: true; text: qsTr("Theme"); color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                Text { Layout.fillWidth: true; text: qsTr("Applies across the whole window."); color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                            }
                            SegControl {
                                id: themeSeg
                                objectName: "themeSeg"
                                Layout.fillWidth: false
                                options: [qsTr("Light"), qsTr("Dark"), qsTr("System")]
                                currentIndex: Theme.mode === "dark" ? 1 : (Theme.mode === "system" ? 2 : 0)
                                onActivated: Theme.mode = index === 0 ? "light" : (index === 1 ? "dark" : "system")
                            }
                        }
                    }
                }
            }

            // ---- Language ----
            Card {
                Layout.fillWidth: true
                implicitHeight: lang.implicitHeight + 40
                ColumnLayout {
                    id: lang
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 16
                    SectionLabel { text: qsTr("Language") }
                    Item {
                        Layout.fillWidth: true
                        implicitHeight: langRow.implicitHeight
                        RowLayout {
                            id: langRow
                            anchors.fill: parent
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Text { Layout.fillWidth: true; text: qsTr("Display language"); color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                Text { Layout.fillWidth: true; text: qsTr("Changes menus, labels and messages across the app."); color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                            }
                            ComboBox {
                                id: langCombo
                                // Fixed min/max, not just preferredWidth: this
                                // Control's own implicit-size calculation
                                // (from its heavily-customized background/
                                // contentItem/indicator) can otherwise win an
                                // initial-layout race against RowLayout's
                                // fillWidth allocation and leave the control
                                // stuck off the row's trailing edge - the same
                                // class of bug fixed in NavItem's badge.
                                Layout.fillWidth: false
                                Layout.minimumWidth: 160
                                Layout.preferredWidth: 160
                                Layout.maximumWidth: 160
                                textRole: "name"
                                valueRole: "code"
                                model: languageManager.availableLanguages()
                                onActivated: languageManager.setLanguage(currentValue)

                                // Set once the model above is guaranteed to be
                                // populated, rather than as a property binding:
                                // indexOfValue() depends on the model but isn't
                                // itself tracked as a binding dependency, so a
                                // `currentIndex: indexOfValue(...)` binding can
                                // evaluate to -1 if it runs before `model` is
                                // applied, leaving the box permanently blank.
                                Component.onCompleted: currentIndex = indexOfValue(languageManager.currentLanguage)
                                Connections {
                                    target: languageManager
                                    function onCurrentLanguageChanged() {
                                        langCombo.currentIndex = langCombo.indexOfValue(languageManager.currentLanguage)
                                    }
                                }

                                font.family: Theme.sans
                                font.pixelSize: 14

                                background: Rectangle {
                                    implicitHeight: 40
                                    radius: Theme.radiusControl
                                    color: Theme.bg
                                    border.width: 1
                                    border.color: langCombo.activeFocus || langCombo.popup.visible ? Theme.accent : Theme.border
                                }
                                contentItem: Text {
                                    // Anchors (not manual x/padding) so this
                                    // flips correctly under LayoutMirroring
                                    // for RTL languages - the indicator ends
                                    // up on the visual left, text anchored
                                    // from the right, matching every other
                                    // mirrored control in the app.
                                    anchors.left: parent.left
                                    anchors.leftMargin: 12
                                    anchors.right: langCombo.indicator.left
                                    anchors.rightMargin: 8
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: langCombo.displayText
                                    font: langCombo.font
                                    color: Theme.ink
                                    verticalAlignment: Text.AlignVCenter
                                    elide: Text.ElideRight
                                }
                                indicator: Text {
                                    anchors.right: parent.right
                                    anchors.rightMargin: 12
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: "⌄"
                                    font.family: Theme.sans
                                    font.pixelSize: 16
                                    color: Theme.muted
                                }
                                popup: Popup {
                                    y: langCombo.height + 4
                                    width: langCombo.width
                                    implicitHeight: contentItem.implicitHeight
                                    padding: 4

                                    contentItem: ListView {
                                        clip: true
                                        implicitHeight: contentHeight
                                        model: langCombo.popup.visible ? langCombo.delegateModel : null
                                        currentIndex: langCombo.highlightedIndex
                                        ScrollIndicator.vertical: ScrollIndicator {}
                                    }
                                    background: Rectangle {
                                        color: Theme.panel
                                        radius: Theme.radiusControl
                                        border.width: 1
                                        border.color: Theme.border
                                    }
                                }
                                delegate: ItemDelegate {
                                    width: langCombo.width
                                    highlighted: langCombo.highlightedIndex === index

                                    contentItem: Text {
                                        leftPadding: 12
                                        text: modelData.name
                                        color: Theme.ink
                                        font.family: Theme.sans
                                        font.pixelSize: 14
                                        verticalAlignment: Text.AlignVCenter
                                    }
                                    background: Rectangle {
                                        color: highlighted ? Theme.accentSoft : "transparent"
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // ---- Solver ----
            Card {
                Layout.fillWidth: true
                implicitHeight: so.implicitHeight + 40
                ColumnLayout {
                    id: so
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 20
                    SectionLabel { text: qsTr("Solver") }

                    Item {
                        Layout.fillWidth: true
                        implicitHeight: minLenRow.implicitHeight
                        RowLayout {
                            id: minLenRow
                            anchors.fill: parent
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Text { Layout.fillWidth: true; text: qsTr("Minimum word length"); color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                Text { Layout.fillWidth: true; text: qsTr("Shortest words shown in the letters game."); color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                            }
                            SegControl {
                                Layout.fillWidth: false
                                options: [qsTr("3+"), qsTr("4+"), qsTr("5+")]
                                currentIndex: AppState.minLen - 3
                                onActivated: AppState.minLen = index + 3
                            }
                        }
                    }

                    Item {
                        Layout.fillWidth: true
                        implicitHeight: flagRow.implicitHeight
                        RowLayout {
                            id: flagRow
                            anchors.fill: parent
                            ColumnLayout {
                                Layout.fillWidth: true; spacing: 2
                                Text { Layout.fillWidth: true; text: qsTr("Flag when no exact answer"); color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                Text { Layout.fillWidth: true; text: qsTr("Highlight the closest result in the numbers game."); color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                            }
                            Switch {
                                id: flagSwitch
                                Layout.fillWidth: false
                                // Switch's built-in implicitWidth is computed from
                                // background/content/padding and knows nothing about
                                // a custom indicator, so it doesn't shrink to fit ours
                                // (40x22) - it stays at its own padded default and
                                // leaves the indicator pinned at x:0, short of the
                                // control's true right edge. Pinning the control's own
                                // box to the indicator size and zeroing padding makes
                                // the two coincide, so it lines up flush with the
                                // other controls above/below it.
                                implicitWidth: 40; implicitHeight: 22
                                padding: 0
                                checked: AppState.flagInexact
                                onToggled: AppState.flagInexact = checked
                                background: Item {}
                                indicator: Rectangle {
                                    anchors.fill: parent
                                    radius: 11
                                    color: flagSwitch.checked ? Theme.accent : Theme.bg
                                    border.width: flagSwitch.checked ? 0 : 1
                                    border.color: Theme.tileBorder
                                    Rectangle {
                                        width: 16; height: 16; radius: 8
                                        anchors.verticalCenter: parent.verticalCenter
                                        x: flagSwitch.checked ? parent.width - width - 3 : 3
                                        color: flagSwitch.checked ? Theme.accentInk : Theme.faint
                                        Behavior on x { NumberAnimation { duration: 120; easing.type: Easing.InOutQuad } }
                                    }
                                }
                            }
                        }
                    }

                    ColumnLayout {
                        Layout.fillWidth: true; spacing: 12
                        Item {
                            Layout.fillWidth: true
                            implicitHeight: maxWordsRow.implicitHeight
                            RowLayout {
                                id: maxWordsRow
                                anchors.fill: parent
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 2
                                    Text { Layout.fillWidth: true; text: qsTr("Max words shown"); color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                                    Text { Layout.fillWidth: true; text: qsTr("Cap the letters-game result list."); color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13 }
                                }
                                Rectangle {
                                    radius: Theme.radiusControl
                                    color: Theme.accentSoft
                                    implicitWidth: maxWordsValue.implicitWidth + 20
                                    implicitHeight: 28
                                    Text {
                                        id: maxWordsValue
                                        anchors.centerIn: parent
                                        text: AppState.maxResults
                                        color: Theme.accent; font.family: Theme.mono
                                        font.pixelSize: 15; font.weight: Font.DemiBold
                                    }
                                }
                            }
                        }
                        Slider {
                            id: wordsSlider
                            Layout.fillWidth: true
                            from: 20; to: 150; stepSize: 10
                            value: AppState.maxResults
                            onMoved: AppState.maxResults = value
                            background: Rectangle {
                                x: wordsSlider.leftPadding
                                y: wordsSlider.topPadding + wordsSlider.availableHeight / 2 - height / 2
                                width: wordsSlider.availableWidth; height: 4; radius: 2
                                color: Theme.border
                                Rectangle {
                                    width: wordsSlider.visualPosition * parent.width
                                    height: parent.height; radius: 2
                                    color: Theme.accent
                                }
                            }
                            handle: Rectangle {
                                x: wordsSlider.leftPadding + wordsSlider.visualPosition * (wordsSlider.availableWidth - width)
                                y: wordsSlider.topPadding + wordsSlider.availableHeight / 2 - height / 2
                                width: 14; height: 14; radius: 3
                                color: Theme.accent
                            }
                        }
                    }
                }
            }

            // ---- Dictionary ----
            Card {
                Layout.fillWidth: true
                implicitHeight: di.implicitHeight + 40
                ColumnLayout {
                    id: di
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 16
                    SectionLabel { text: qsTr("Dictionary") }
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            Layout.fillWidth: true; spacing: 2
                            Text { Layout.fillWidth: true; text: qsTr("Word list"); color: Theme.ink; font.family: Theme.sans; font.pixelSize: 15; font.weight: Font.DemiBold }
                            Text {
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                                // solver.dictionariesReady is read here (and
                                // below) purely so this binding re-evaluates
                                // once the deferred dictionary load
                                // completes - fullDictionaryAvailable() alone
                                // is a plain invokable QML can't track.
                                // languageManager.currentLanguage is read for
                                // the same reason on a language switch:
                                // fullDictionaryStatus()'s text is translated
                                // in C++, and engine.retranslate() only
                                // re-evaluates qsTr() calls written directly
                                // in QML, not the return value of a plain
                                // invokable - without this read, the status
                                // text would keep showing whatever language
                                // was active when this binding last ran.
                                text: (solver.dictionariesReady && solver.fullDictionaryAvailable())
                                      ? qsTr("Swap the built-in dictionary for your custom words.txt list.")
                                      : (languageManager.currentLanguage, solver.fullDictionaryStatus())
                                color: Theme.muted; font.family: Theme.sans; font.pixelSize: 13
                            }
                        }
                        SegControl {
                            id: dictSeg
                            objectName: "dictSeg"
                            Layout.fillWidth: false
                            options: [qsTr("Default"), qsTr("Custom")]
                            enabled: solver.dictionariesReady && solver.fullDictionaryAvailable()
                            opacity: enabled ? 1 : 0.5
                            currentIndex: AppState.useFullDictionary ? 1 : 0
                            onActivated: {
                                const wantFull = index === 1
                                if (solver.setUseFullDictionary(wantFull)) {
                                    AppState.useFullDictionary = wantFull
                                } else {
                                    dictSeg.currentIndex = AppState.useFullDictionary ? 1 : 0
                                }
                            }
                        }
                    }
                }
            }

            // ---- About ----
            Card {
                Layout.fillWidth: true
                implicitHeight: ab.implicitHeight + 40
                ColumnLayout {
                    id: ab
                    anchors.fill: parent; anchors.margins: 20
                    spacing: 12
                    SectionLabel { text: qsTr("About") }
                    ColumnLayout {
                        id: aboutText
                        objectName: "aboutText"
                        Layout.fillWidth: true
                        spacing: 4
                        readonly property var versionLines: solver.versionDetails().split("\n")
                        Text {
                            Layout.fillWidth: true
                            text: aboutText.versionLines[0] || ""
                            color: Theme.ink; font.family: Theme.sans
                            font.pixelSize: 15; font.weight: Font.DemiBold
                        }
                        Text {
                            Layout.fillWidth: true
                            visible: text.length > 0
                            text: aboutText.versionLines.slice(1).join("\n")
                            color: Theme.muted; font.family: Theme.mono
                            font.pixelSize: 12
                            wrapMode: Text.WordWrap
                        }
                    }
                    Text {
                        id: repoLink
                        Layout.topMargin: 4
                        text: qsTr("View on GitHub ↗")
                        color: repoLinkArea.containsMouse ? Theme.ink : Theme.accent
                        font.family: Theme.sans; font.pixelSize: 13; font.weight: Font.DemiBold
                        MouseArea {
                            id: repoLinkArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: Qt.openUrlExternally("https://github.com/iainchesworth/CountdownSolver")
                        }
                    }
                    Text {
                        id: licensesLink
                        objectName: "licensesLink"
                        text: "Third-Party Licenses"
                        color: licensesLinkArea.containsMouse ? Theme.ink : Theme.accent
                        font.family: Theme.sans; font.pixelSize: 13; font.weight: Font.DemiBold
                        MouseArea {
                            id: licensesLinkArea
                            anchors.fill: parent
                            hoverEnabled: true
                            cursorShape: Qt.PointingHandCursor
                            onClicked: licensesDialog.open()
                        }
                    }
                }
            }

            LicensesDialog { id: licensesDialogInstance }

            Item { Layout.preferredHeight: 4 }
        }
    }
}
