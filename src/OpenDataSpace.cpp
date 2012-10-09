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
 */
#include "OpenDataSpace.hpp"
#include "FileInfo.hpp"
#include "DateUtil.hpp"

#include <bb/system/InvokeManager.hpp>
#include <bb/system/InvokeRequest.hpp>
#include <bb/system/SystemCredentialsPrompt>
#include <bb/system/SystemDialog>
#include <bb/system/SystemPrompt>
#include <bb/system/SystemToast>
#include <bb/system/SystemUiButton>
#include <bb/system/SystemUiError>
#include <bb/system/SystemUiInputField>
#include <bb/system/SystemUiInputMode>
#include <bb/system/SystemUiPosition>
#include <bb/system/SystemUiResult>

#include <bb/cascades/Application>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/LocaleHandler>
#include <bb/cascades/Menu>
#include <bb/cascades/ActionItem>
#include <bb/cascades/HelpActionItem>
#include <bb/cascades/SettingsActionItem>
#include <bb/cascades/Sheet>

#include <bb/cascades/pickers/FilePicker>
#include <bb/cascades/pickers/FilePickerMode>
#include <bb/cascades/pickers/FilePickerSortFlag>
#include <bb/cascades/pickers/FilePickerSortOrder>
#include <bb/cascades/pickers/FileType>
#include <bb/cascades/pickers/ViewMode>

#include <bps/virtualkeyboard.h>

using namespace bb::cascades;
using namespace bb::system;

/*
 *
 * Author: Ekkehard Gentz (ekke), Rosenheim, Germany
 *
 */
OpenDataSpace::OpenDataSpace() {

	// register the MyListModel C++ type to be visible in QML

	// Register some classes for Filepicker for QML
	qmlRegisterType<bb::cascades::pickers::FilePicker>("bb.cascades.pickers", 1,
			0, "FilePicker");
	qmlRegisterUncreatableType<bb::cascades::pickers::FilePickerMode>(
			"bb.cascades.pickers", 1, 0, "FilePickerMode", "");
	qmlRegisterUncreatableType<bb::cascades::pickers::FilePickerSortFlag>(
			"bb.cascades.pickers", 1, 0, "FilePickerSortFlag", "");
	qmlRegisterUncreatableType<bb::cascades::pickers::FilePickerSortOrder>(
			"bb.cascades.pickers", 1, 0, "FilePickerSortOrder", "");
	qmlRegisterUncreatableType<bb::cascades::pickers::FileType>(
			"bb.cascades.pickers", 1, 0, "FileType", "");
	qmlRegisterUncreatableType<bb::cascades::pickers::ViewMode>(
			"bb.cascades.pickers", 1, 0, "ViewMode", "");

	// Register the FileInfo, so QML knows about
	qmlRegisterType<FileInfo>("org.opendataspace.fileinfo", 1, 0, "FileInfo");

	// Register the DateUtil, so QML knows about
	qmlRegisterType<DateUtil>("org.opendataspace.dateutil", 1, 0, "DateUtil");

	// register system dialogs
	qmlRegisterType<SystemUiButton>("bb.system", 1, 0, "SystemUiButton");
	qmlRegisterType<SystemUiInputField>("bb.system", 1, 0, "SystemUiInputField");
	qmlRegisterType<SystemToast>("bb.system", 1, 0, "SystemToast");
	qmlRegisterType<SystemPrompt>("bb.system", 1, 0, "SystemPrompt");
	qmlRegisterType<SystemCredentialsPrompt>("bb.system", 1, 0, "SystemCredentialsPrompt");
	qmlRegisterType<SystemDialog>("bb.system", 1, 0, "SystemDialog");
	qmlRegisterUncreatableType<SystemUiError>("bb.system", 1, 0, "SystemUiError", "");
	qmlRegisterUncreatableType<SystemUiResult>("bb.system", 1, 0, "SystemUiResult", "");
	qmlRegisterUncreatableType<SystemUiPosition>("bb.system", 1, 0, "SystemUiPosition", "");
	qmlRegisterUncreatableType<SystemUiInputMode>("bb.system", 1, 0, "SystemUiInputMode", "");
	qmlRegisterUncreatableType<SystemUiModality>("bb.system", 1, 0, "SystemUiModality", "");
	qRegisterMetaType<SystemUiResult::Type>( "bb::system::SystemUiResult::Type");

	qDebug() << "registered types for QML";

	// our main QML document: the HomeScreen with a custom Background Image
	QmlDocument *qml = QmlDocument::create("asset:///main.qml").parent(this);
	qDebug() << "created QML Document";

	//-- setContextProperty expose C++ object in QML as an variable
	qml->setContextProperty("ods", this);

	// create root object for the UI
	AbstractPane *root = qml->createRootObject<AbstractPane>();
	qDebug() << "created root object";

	Application::instance()->setScene(root);
	qDebug() << "set the scene";

	// ApplicationMenu
	// Hint: first set the scene - then set the menu
	Menu* menu = createApplicationMenu();
	qDebug() << "created ApplicartionMenu";

	Application::instance()->setMenu(menu);
	qDebug() << "set ApplicationMenu";

	// first translation
	translateMenuItems();
	qDebug() << "did first translations";

	//
	Application *app = Application::instance();
	connect(app, SIGNAL(thumbnail()), this, SLOT(onThumbnail()));

	qDebug() << "INIT done";

}

// INTERNATIONALIZATION (i18n)
/**
 *
 * This method initializes translation engine based on current locale
 * at runtime.
 *
 */
void OpenDataSpace::initLocalization(QTranslator* translator) {
	// remember current locale set
	m_currentLocale = QLocale().name();
	qDebug() << "init with locale: " << m_currentLocale;
	m_translator = translator;

	// watch if user changes locale from device settings
	m_LocaleHandler = new LocaleHandler(this);
	// connect the handler
	connect(m_LocaleHandler, SIGNAL(systemLanguageChanged()), this,
			SLOT(localeChanged()));
	qDebug() << "connected systemLanguageChanged";

}

/**
 * App::updateLocale(QString locale)
 *
 * Update view content basing on the given locale.
 *
 */
void OpenDataSpace::updateLocale(QString locale) {
	qDebug() << "updateLocale: " << locale;

	// if locale is empty - refresh current. otherwise change the local
	if (!locale.trimmed().isEmpty() && m_currentLocale != locale) {
		m_currentLocale = locale;

		qDebug() << "updating UI to language: " << m_currentLocale;
		QString filename = QString("OpenDataSpace_%1").arg(m_currentLocale);
		if (m_translator->load(filename, "app/native/qm")) {
			// multiple translators can be installed but for this
			// app we only use one translator instance for brevity
			Application::instance()->removeTranslator(m_translator);
			Application::instance()->installTranslator(m_translator);
			// retranslate System menu items
			translateMenuItems();
		}

	}
}

/**
 * (re)translates the titles of System menus
 *
 */
void OpenDataSpace::translateMenuItems() {
	if (m_helpItem) {
		m_helpItem->setTitle(tr("Help"));
	}
	if (m_feedbackItem) {
		m_feedbackItem->setTitle(tr("Feedback"));
	}
	if (m_logoutItem) {
		m_logoutItem->setTitle(tr("Logout"));
	}
	if (m_settingsItem) {
		m_settingsItem->setTitle(tr("Settings"));
	}
}

/**
 * App::getCurrentLanguage()
 *
 * Retrieve the language name corresponding to the current locale.
 */
QString OpenDataSpace::getCurrentLanguage() {
	// TODO get language name from QLocale - we have now more languages
	qDebug() << "OpenDataSpaceApp getCurrentLanguage: " << m_currentLocale;
	QLocale *loc = new QLocale(m_currentLocale);
	return loc->languageToString(loc->language());
}

/**
 * App::getCurrentLocale()
 *
 * Retrieve the current locale.
 */
QString OpenDataSpace::getCurrentLocale() {
	qDebug() << "getCurrentLocale: " << m_currentLocale;
	return m_currentLocale;
}

/**
 * App::suppressKeyboard()
 *
 * A helper function to force the keyboard to hide
 */
void OpenDataSpace::suppressKeyboard() {
	virtualkeyboard_request_events(0);
	virtualkeyboard_hide();
}

// M E N U
// ApplicationMenu is available on all Screens
// opens using swipe-down
Menu* OpenDataSpace::createApplicationMenu() {
	// HELP will open a website with Help Instructions from OpenDataSpace
	m_helpItem = new HelpActionItem();
	// FEEDBACK will send an email to OpenDataSpace
	m_feedbackItem = new ActionItem();
	m_feedbackItem->setImageSource(
			QString("asset:///images/ics/5-content-email81.png"));
	// LOGOUT will do a LogOut and jump back to HomeScreen and open the LogIn Sheet
	m_logoutItem = new ActionItem();
	m_logoutItem->setImageSource(
			QString("asset:///images/ics/10-device-access-accounts81.png"));
	// SETTINGS will open the User Settings
	m_settingsItem = new SettingsActionItem();
	// set the translated Titles
	translateMenuItems();
	// plug it all together
	Menu* menu = Menu::create().addAction(m_feedbackItem).addAction(
			m_logoutItem).help(m_helpItem).settings(m_settingsItem);
	// Connect SIGNALS and SLOTS
	QObject::connect(m_logoutItem, SIGNAL(triggered()), this,
			SLOT(logoutTriggered()));
	QObject::connect(m_feedbackItem, SIGNAL(triggered()), this,
			SLOT(feedbackTriggered()));
	QObject::connect(m_helpItem, SIGNAL(triggered()), this,
			SLOT(helpTriggered()));
	QObject::connect(m_settingsItem, SIGNAL(triggered()), this,
			SLOT(settingsTriggered()));
	return menu;
}

// S L O T S

// handles SLOT from Locale Chaned by user at Device
void OpenDataSpace::localeChanged() {
	updateLocale(QLocale().name());
}

// handles SLOT from logoutItem
void OpenDataSpace::logoutTriggered() {
	Sheet *s = Application::instance()->scene()->findChild<Sheet*>(
			"loginSheet");
	if (s) {
		qDebug() << "logout triggered and loginSheet found";
		s->open();
	} else {
		qDebug() << "logout triggered, but no loginSheet found";
	}
}

// handles SLOT from feedbackItem
void OpenDataSpace::feedbackTriggered() {
	Sheet *s = Application::instance()->scene()->findChild<Sheet*>(
			"feedbackSheet");
	if (s) {
		qDebug() << "feedback triggered and Feedback Sheet found";
		s->open();
	} else {
		qDebug() << "feedback triggered, but no Feedback Sheet found";
	}
}

// handles SLOT from helpItem
void OpenDataSpace::helpTriggered() {
	Sheet *s = Application::instance()->scene()->findChild<Sheet*>("helpSheet");
	if (s) {
		qDebug() << "help triggered and helpSheet found";
		s->open();
	} else {
		qDebug() << "help triggered, but no helpSheet found";
	}
}

// handles SLOT from settingsItem
void OpenDataSpace::settingsTriggered() {
	Sheet *s = Application::instance()->scene()->findChild<Sheet*>(
			"preferencesSheet");
	if (s) {
		qDebug() << "preferences triggered and preferencesSheet found";
		s->open();

	} else {
		qDebug() << "preferences triggered, but no FpreferencesSheet found";
	}
}

/**
 * uses Invokation Framework to View the file from URI
 *
 */
void OpenDataSpace::showInView(QString uri) {
	qDebug() << "showInView called: " << uri;
	InvokeRequest invokeRequest;
	invokeRequest.setAction("bb.action.VIEW");
	invokeRequest.setUri(uri);
	qDebug() << "showInView URI: " << invokeRequest.uri();
	InvokeManager invokeManager;
	invokeManager.invoke(invokeRequest);
}

/**
 * uses Invokation Framework to View the file from URI
 * for a specific MimeType
 *
 */
void OpenDataSpace::showInViewForMimeType(QString uri, QString mimeType) {
	qDebug() << "showInViewForMimeType called: " << uri;
	InvokeRequest invokeRequest;
	invokeRequest.setAction("bb.action.VIEW");
	invokeRequest.setUri(uri);
	invokeRequest.setMimeType(mimeType);
	qDebug() << "showInViewForMimeType URI: " << invokeRequest.uri() << " Mime:"
			<< mimeType;
	InvokeManager invokeManager;
	invokeManager.invoke(invokeRequest);
}

/**
 * uses Invokation Framework to View the file from URI
 * for a specific target like "sys.pictures.app"
 *
 */
void OpenDataSpace::showInTarget(QString uri, QString target) {
	qDebug() << "showInTarget called: " << uri;
	InvokeRequest invokeRequest;
	invokeRequest.setAction("bb.action.VIEW");
	invokeRequest.setUri(uri);
	invokeRequest.setTarget(target);
	qDebug() << "showInTarget URI: " << invokeRequest.uri();
	InvokeManager invokeManager;
	invokeManager.invoke(invokeRequest);
}

/**
 * uses Invokation Framework to View the file from URI
 * for a specific MimeType
 * and for a specific target like "sys.pictures.app"
 *
 */
void OpenDataSpace::showInTargetForMimeType(QString uri, QString mimeType,
		QString target) {
	qDebug() << "showInTargetForMimeType called: " << uri;
	InvokeRequest invokeRequest;
	invokeRequest.setAction("bb.action.VIEW");
	invokeRequest.setUri(uri);
	invokeRequest.setTarget(target);
	invokeRequest.setMimeType(mimeType);
	qDebug() << "showInTargetForMimeType URI: " << invokeRequest.uri()
			<< " MimeType:" << mimeType;
	InvokeManager invokeManager;
	invokeManager.invoke(invokeRequest);
}

// triggered if Application was sent to back
void OpenDataSpace::onThumbnail() {
	// TODO set Cover()
	qDebug() << "Application shrinks to thumbnail";
	// AbstractCover *cover;
	// Application::instance()->setCover(cover);
}
