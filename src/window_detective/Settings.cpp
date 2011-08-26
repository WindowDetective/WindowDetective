/////////////////////////////////////////////////////////////////////
// File: Settings.xpp                                              //
// Date: 26/2/10                                                   //
// Desc: Handles reading and writing settings that are stored in   //
//   the Windows registry. Values are written when the application //
//   exits or when the user changes them in the preferences.       //
//   If the application is run on a machine which it is not        //
//   installed on, no settings are saved to the registry unless    //
//   specified by the user.                                        //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2011 XTAL256

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "Settings.h"
#include "window_detective/main.h"
#include "window_detective/Logger.h"
#include "window_detective/QtHelpers.h"


int Settings::appInstalled = -1;

bool Settings::use32bitCursor;
bool Settings::canPickTransparentWindows;
bool Settings::hideWhilePicking;
bool Settings::stayOnTop;
bool Settings::allowInspectOwnWindows;
uint Settings::messageTimeoutPeriod;
QRegExp::PatternSyntax Settings::regexType;
String Settings::appStyle;
bool Settings::greyHiddenWindows;
uint Settings::treeChangeDuration;
QColor Settings::itemCreatedColourImmediate;
QColor Settings::itemCreatedColourUnexpanded;
QColor Settings::itemDestroyedColourImmediate;
QColor Settings::itemDestroyedColourUnexpanded;
QColor Settings::itemChangedColourImmediate;
QColor Settings::itemChangedColourUnexpanded;
QColor Settings::highlighterColour;
HighlightStyle Settings::highlighterStyle;
int Settings::highlighterBorderThickness;
QStringList Settings::infoLabels;
bool Settings::enableLogging;
bool Settings::enableBalloonNotifications;
String Settings::logOutputFolder;

/*------------------------------------------------------------------+
| If the app's registry key does not exist (because this app was    |
| not installed), then don't create it. No settings will be saved.  |
+------------------------------------------------------------------*/
bool Settings::isAppInstalled() {
    if (appInstalled != -1)
        return (bool)appInstalled; // Cached result

    HKEY key;
    LONG result = RegOpenKeyEx(HKEY_CURRENT_USER,
                               L"Software\\Window Detective", 0,
                               KEY_QUERY_VALUE, &key);
    appInstalled = ((result == ERROR_SUCCESS) ? 1 : 0);
    return (bool)appInstalled;
}

/*------------------------------------------------------------------+
| Sets up default values for settings.                              |
+------------------------------------------------------------------*/
void Settings::initialize() {
    use32bitCursor = true;
    canPickTransparentWindows = false;
    hideWhilePicking = true;
    stayOnTop = false;
    allowInspectOwnWindows = false;
    messageTimeoutPeriod = 500;
    regexType = QRegExp::RegExp;
    appStyle = "native";
    greyHiddenWindows = false;
    treeChangeDuration = 500;
    itemCreatedColourImmediate = QColor(0,255,0);
    itemCreatedColourUnexpanded = QColor(128,255,128);
    itemDestroyedColourImmediate = QColor(255,0,0);
    itemDestroyedColourUnexpanded = QColor(255,128,128);
    itemChangedColourImmediate = QColor(0,0,0);
    itemChangedColourUnexpanded = QColor(80,80,80);
    highlighterColour = Qt::red;
    highlighterStyle = Border;
    highlighterBorderThickness = 4;
    infoLabels = String("windowClass,handle,dimensions,size").split(",");
    enableLogging = false;
    enableBalloonNotifications = true;
    logOutputFolder = "";
}

/*------------------------------------------------------------------+
| Reads the settings from the registry (under HKCU\Software).       |
| If the registry key does not exist, then don't try to read.       |
+------------------------------------------------------------------*/
void Settings::read() {
    if (!isAppInstalled())
        return;
    QSettings registrySettings(APP_NAME, APP_NAME);
    read(registrySettings);
}

/*------------------------------------------------------------------+
| Writes the settings to the registry. If the Window Detective      |
| registry key does not already exist, then it won't be written to, |
| as the application probably isn't installed.                      |
+------------------------------------------------------------------*/
void Settings::write() {
    if (!isAppInstalled())
        return;
    QSettings registrySettings(APP_NAME, APP_NAME);
    write(registrySettings);
}

/*------------------------------------------------------------------+
| Reads (imports) the settings from the given INI file.             |
+------------------------------------------------------------------*/
void Settings::read(const String& fileName) {
    QSettings iniSettings(fileName, QSettings::IniFormat);
    read(iniSettings);
}

/*------------------------------------------------------------------+
| Writes (exports) the settings to the given INI file. If the file  |
| does not exist, it will be created.                               |
+------------------------------------------------------------------*/
void Settings::write(String& fileName) {
    QSettings iniSettings(fileName, QSettings::IniFormat);
    write(iniSettings);
}

/*------------------------------------------------------------------+
| This function does the actual reading of values, either from the  |
| registry or from an INI file.                                     |
+------------------------------------------------------------------*/
void Settings::read(const QSettings& settings) {
    // Only use 32bit cursor if running XP or higher. Else, force 16bit cursor
    if (getOSVersion() >= 501) {
        use32bitCursor = settings.value("use32bitCursor", true).toBool();
    }
    else {
        use32bitCursor = false;
    }

    /* TODO:
        if (contains("name")
            variable = value("name").toBool();
    */
    canPickTransparentWindows = settings.value("canPickTransparentWindows", false).toBool();
    hideWhilePicking = settings.value("hideWhilePicking", true).toBool();
    stayOnTop = settings.value("stayOnTop", false).toBool();
    allowInspectOwnWindows = settings.value("allowInspectOwnWindows", false).toBool();
    messageTimeoutPeriod = settings.value("messageTimeoutPeriod", 500).toUInt();
    regexType = static_cast<QRegExp::PatternSyntax>(settings.value("regexType", QRegExp::RegExp).toUInt());
    appStyle = settings.value("applicationStyle", "native").toString();

    greyHiddenWindows = settings.value("tree/greyHiddenWindows", false).toBool();
    treeChangeDuration = settings.value("tree/changeDuration", 500).toUInt();
    itemCreatedColourImmediate = stringToColour(settings.value("tree/itemCreatedColourImmediate", "0,255,0").toString());
    itemCreatedColourUnexpanded = stringToColour(settings.value("tree/itemCreatedColourUnexpanded", "128,255,128").toString());
    itemDestroyedColourImmediate = stringToColour(settings.value("tree/itemDestroyedColourImmediate", "255,0,0").toString());
    itemDestroyedColourUnexpanded = stringToColour(settings.value("tree/itemDestroyedColourUnexpanded", "255,128,128").toString());
    itemChangedColourImmediate = stringToColour(settings.value("tree/itemChangedColourImmediate", "0,0,0").toString());
    itemChangedColourUnexpanded = stringToColour(settings.value("tree/itemChangedColourUnexpanded", "80,80,80").toString());

    highlighterColour = stringToColour(settings.value("highlighter/colour", "255,0,0").toString());
    highlighterStyle = static_cast<HighlightStyle>(settings.value("highlighter/style", Border).toInt());
    highlighterBorderThickness = settings.value("highlighter/borderThickness", 4).toInt();

    String defaultLabels = "windowClass,handle,dimensions,size";
    String infoLabelString = settings.value("infoWindow/labels", defaultLabels).toString();
    infoLabels = infoLabelString.split(",");

    enableLogging = settings.value("logging/enable", false).toBool();
    enableBalloonNotifications = settings.value("logging/enableBalloonNotifications", true).toBool();
    logOutputFolder = settings.value("logging/outputFolder", "").toString();
}

/*------------------------------------------------------------------+
| This function does the actual writing of values, either to the    |
| registry or to an INI file.                                       |
+------------------------------------------------------------------*/
void Settings::write(QSettings& settings) {
    settings.setValue("use32bitCursor", use32bitCursor);
    settings.setValue("canPickTransparentWindows", canPickTransparentWindows);
    settings.setValue("hideWhilePicking", hideWhilePicking);
    settings.setValue("stayOnTop", stayOnTop);
    settings.setValue("allowInspectOwnWindows", allowInspectOwnWindows);
    settings.setValue("messageTimeoutPeriod", messageTimeoutPeriod);
    settings.setValue("regexType", static_cast<int>(regexType));
    settings.setValue("applicationStyle", appStyle);

    settings.setValue("tree/greyHiddenWindows", greyHiddenWindows);
    settings.setValue("tree/changeDuration", treeChangeDuration);
    settings.setValue("tree/itemCreatedColourImmediate", colourToString(itemCreatedColourImmediate));
    settings.setValue("tree/itemCreatedColourUnexpanded", colourToString(itemCreatedColourUnexpanded));
    settings.setValue("tree/itemDestroyedColourImmediate", colourToString(itemDestroyedColourImmediate));
    settings.setValue("tree/itemDestroyedColourUnexpanded", colourToString(itemDestroyedColourUnexpanded));
    settings.setValue("tree/itemChangedColourImmediate", colourToString(itemChangedColourImmediate));
    settings.setValue("tree/itemChangedColourUnexpanded", colourToString(itemChangedColourUnexpanded));

    settings.setValue("highlighter/colour", colourToString(highlighterColour));
    settings.setValue("highlighter/style", static_cast<int>(highlighterStyle));
    settings.setValue("highlighter/borderThickness", highlighterBorderThickness);

    String infoLabelString = infoLabels.join(",");
    settings.setValue("infoWindow/labels", infoLabelString);

    settings.setValue("logging/enable", enableLogging);
    settings.setValue("logging/enableBalloonNotifications", enableBalloonNotifications);
    settings.setValue("logging/outputFolder", logOutputFolder);
}
