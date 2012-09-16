/*
 * Copyright (c) 2012 SSP Europe GmbH, Munich
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */import bb.cascades 1.0
import "common"

/*
 * Preferences and Options set by the user
 * 
 * Author: Ekkehard Gentz (ekke), Rosenheim, Germany
 *
*/

// we're using a NavigationPane to be able to push/pop Pages with special editors like language selection

NavigationPane {
    // SIGNAL
    signal done(bool ok)
    // the current locale - can be set from outside
    property string currentLanguage: "en"
    // the current customer - can be set from outside
    property string currentCustomer: "Musterfirma GmbH"
    id: navigationPane
    attachedObjects: [
        // special editor to select the Language
        LanguageSelection {
            id: languageSelection
            paneProperties: NavigationPaneProperties {
                backButton: ActionItem {
                    onTriggered: {
                        navigationPane.pop();
                    }
                }
            }
        },
        // special editor to select the Customer
        CustomerSelection {
            id: customerSelection
            paneProperties: NavigationPaneProperties {
                backButton: ActionItem {
                    onTriggered: {
                        navigationPane.pop();
                    }
                }
            }
        }
    ]
    Page {
        
        // seems not to work
        resizeBehavior: PageResizeBehavior.Resize
        titleBar: TitleBar {
            id: theBar
            title: qsTr("Preferences") + Retranslate.onLanguageChanged
            visibility: ChromeVisibility.Visible
            dismissAction: ActionItem {
                title: qsTr("Cancel") + Retranslate.onLanguageChanged
                onTriggered: {
                    navigationPane.done(false);
                }
            }
            acceptAction: ActionItem {
                title: qsTr("Save") + Retranslate.onLanguageChanged
                onTriggered: {
                    navigationPane.done(true);
                }
            }
        }
        // 
        Container {
            layout: DockLayout {
            }
            id: mainContainer
            //ScrollView {
            Container {
                id: fieldsContainer
                layout: StackLayout {
                    topPadding: 25
                    leftPadding: 25
                    rightPadding: 25
                    layoutDirection: LayoutDirection.TopToBottom
                }
                TextField {
                    id: server
                    hintText: qsTr("Server URL") + Retranslate.onLanguageChanged
                    inputMode: TextFieldInputMode.URL
                    textStyle {
                        base: SystemDefaults.TextStyles.BodyText
                    }
                }
                TextField {
                    id: email
                    hintText: qsTr("User email address") + Retranslate.onLanguageChanged
                    inputMode: TextFieldInputMode.EmailAddress
                    textStyle {
                        base: SystemDefaults.TextStyles.BodyText
                    }
                }
                // Label displays the current selected customer
                Label {
                    id: customerLabel
                    text: "Musterfirma GmbH"
                    leftMargin: 20
                    textStyle {
                        base: SystemDefaults.TextStyles.BodyText
                    }
                    // open the language editor
                    onTouch: {
                        customerSelection.setCustomer(navigationPane.currentCustomer)
                        navigationPane.push(customerSelection)
                    }
                }
                
                // Label displays the current Locale
                Label {
                    id: languageLabel
                    //gets the actual language but too early, so its always english
                    // TODO
                    text: ods.getCurrentLanguage()
                    leftMargin: 20
                    textStyle {
                        base: SystemDefaults.TextStyles.BodyText
                    }
                    // open the language editor
                    onTouch: {
                        languageSelection.setLanguage(navigationPane.currentLanguage)
                        navigationPane.push(languageSelection)
                    }
                }
                Divider {
                    topMargin: 20
                    bottomMargin: 20
                }
                // TODO data from server
                Label {
                    id: memoryUsedLabel
                    text: "257.0 MB " + qsTr("of") + Retranslate.onLanguageChanged + " 2.0 GB " + qsTr("Memory used") + Retranslate.onLanguageChanged
                    leftMargin: 20
                    textStyle {
                        base: SystemDefaults.TextStyles.BodyText
                    }
                }
                // special class to draw a bar with red and green colors
                RedGreenBar {
                    id: usedMem
                }
                Label {
                    id: uploadLabel
                    text: "86.6 MB " + qsTr("Files to upload") + Retranslate.onLanguageChanged
                    leftMargin: 20
                    textStyle {
                        base: SystemDefaults.TextStyles.BodyText
                    }
                }
                RedGreenBar {
                    id: afterUploadMem
                }
                Label {
                    id: versionLabel
                    text: qsTr("Version %1").arg("1.3.2") + Retranslate.onLanguageChanged
                    leftMargin: 20
                    textStyle {
                        base: SystemDefaults.TextStyles.SmallText
                    }
                }
            }
            //} // ScrollView
        }
    }
    // SLOTS
    function newLanguage(locale) {
        console.debug("new locale: " + locale)
        navigationPane.currentLanguage = locale
        if (locale == "de" || locale == "de_DE") {
            languageLabel.text = qsTr("German") + Retranslate.onLanguageChanged
        } else if (locale == "fr") {
            languageLabel.text = qsTr("French") + Retranslate.onLanguageChanged
        } else if (locale == "es") {
            languageLabel.text = qsTr("Spanish") + Retranslate.onLanguageChanged
        } else if (locale == "it") {
            languageLabel.text = qsTr("Italian") + Retranslate.onLanguageChanged
        } else if (locale == "ru") {
            languageLabel.text = qsTr("Russian") + Retranslate.onLanguageChanged
        } else {
            languageLabel.text = qsTr("English") + Retranslate.onLanguageChanged
        }
    }
    function newCustomer(name) {
        console.debug("new customer: " + name)
        navigationPane.currentCustomer = name
        customerLabel.text = name
    }
    onCreationCompleted: {
        //-- connect the preferences save SIGNAL to the handler SLOT
        languageSelection.languageChanged.connect(navigationPane.newLanguage)
        customerSelection.customerChanged.connect(navigationPane.newCustomer)
        // init values
        usedMem.setRedGreen(257, 2000);
        afterUploadMem.setRedGreen(344, 2000);
    }
}
