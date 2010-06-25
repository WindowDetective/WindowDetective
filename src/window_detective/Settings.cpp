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

#include "Settings.h"
#include "window_detective/main.h"
#include "window_detective/Logger.h"


int Settings::appInstalled = -1;

bool Settings::use32bitCursor;
bool Settings::canPickTransparentWindows;
bool Settings::hideWhilePicking;
uint Settings::messageTimeoutPeriod;
bool Settings::greyHiddenWindows;
QPair<QColor,QColor> Settings::itemCreatedColours;
QPair<QColor,QColor> Settings::itemDestroyedColours;
QPair<QColor,QColor> Settings::itemChangedColours;
uint Settings::treeChangeDuration;
QRegExp::PatternSyntax Settings::regexType;
QColor Settings::highlighterColour;
HighlightStyle Settings::highlighterStyle;
int Settings::highlighterBorderThickness;
QStringList Settings::infoLabels;
bool Settings::enableLogging;
String Settings::logOutputFolder;
String Settings::appStyle;
bool Settings::allowInspectOwnWindows;


/*------------------------------------------------------------------+
| If the app's registry key does not exist (because this app was    |
| not installed), then don't create it. No settings will be saved.  |
+------------------------------------------------------------------*/
bool Settings::isAppInstalled() {
    if (appInstalled != -1)
        return (bool)appInstalled; // Cached result

    HKEY key;
    LONG result = RegOpenKey(HKEY_CURRENT_USER, L"Software\\Window Detective", &key);
    appInstalled = ((result == ERROR_SUCCESS) ? 1 : 0);
    return (bool)appInstalled;
}

void Settings::read() {
    String regName = isAppInstalled() ? APP_NAME : "";
    QSettings reg(regName, regName);

    // Only use 32bit cursor if running XP or higher. Else, force 16bit cursor
    if (getOSVersion() >= 501)
        use32bitCursor = reg.value("use32bitCursor", true).toBool();
    else
        use32bitCursor = false;
    canPickTransparentWindows = reg.value("canPickTransparentWindows", false).toBool();
    hideWhilePicking = reg.value("hideWhilePicking", true).toBool();
    messageTimeoutPeriod = reg.value("messageTimeoutPeriod", 500).toUInt();
    regexType = static_cast<QRegExp::PatternSyntax>(reg.value("regexType", QRegExp::RegExp).toUInt());

    greyHiddenWindows = reg.value("tree/greyHiddenWindows", false).toBool();
    treeChangeDuration = reg.value("tree/changeDuration", 500).toUInt();
    QColor colour1, colour2;
    colour1 = stringToColour(reg.value("tree/itemCreatedColourImmediate", "0,255,0").toString());
    colour2 = stringToColour(reg.value("tree/itemCreatedColourUnexpanded", "128,255,128").toString());
    itemCreatedColours = qMakePair(colour1, colour2);
    colour1 = stringToColour(reg.value("tree/itemDestroyedColourImmediate", "255,0,0").toString());
    colour2 = stringToColour(reg.value("tree/itemDestroyedColourUnexpanded", "255,128,128").toString());
    itemDestroyedColours = qMakePair(colour1, colour2);
    colour1 = stringToColour(reg.value("tree/itemChangedColourImmediate", "0,0,0").toString());
    colour2 = stringToColour(reg.value("tree/itemChangedColourUnexpanded", "80,80,80").toString());
    itemChangedColours = qMakePair(colour1, colour2);

    highlighterColour = stringToColour(reg.value("highlighter/colour", "255,0,0").toString());
    highlighterStyle = static_cast<HighlightStyle>(reg.value("highlighter/style", Border).toInt());
    highlighterBorderThickness = reg.value("highlighter/borderThickness", 4).toInt();

    String defaultLabels = "windowClass,handle,dimensions,size";
    String infoLabelString = reg.value("infoWindow/labels", defaultLabels).toString();
    infoLabels = infoLabelString.split(",");

    enableLogging = reg.value("logging/enable", false).toBool();
    logOutputFolder = reg.value("logging/outputFolder", "").toString();

    appStyle = reg.value("applicationStyle", "native").toString();
    allowInspectOwnWindows = reg.value("allowInspectOwnWindows", false).toBool();
}

void Settings::write() {
    if (!isAppInstalled())
        return;

    QSettings reg;
    reg.setValue("use32bitCursor", use32bitCursor);
    reg.setValue("canPickTransparentWindows", canPickTransparentWindows);
    reg.setValue("hideWhilePicking", hideWhilePicking);
    reg.setValue("messageTimeoutPeriod", messageTimeoutPeriod);
    reg.setValue("regexType", static_cast<int>(regexType));

    reg.setValue("tree/greyHiddenWindows", greyHiddenWindows);
    reg.setValue("tree/changeDuration", treeChangeDuration);
    reg.setValue("tree/itemCreatedColourImmediate", colourToString(itemCreatedColours.first));
    reg.setValue("tree/itemCreatedColourUnexpanded", colourToString(itemCreatedColours.second));
    reg.setValue("tree/itemDestroyedColourImmediate", colourToString(itemDestroyedColours.first));
    reg.setValue("tree/itemDestroyedColourUnexpanded", colourToString(itemDestroyedColours.second));
    reg.setValue("tree/itemChangedColourImmediate", colourToString(itemChangedColours.first));
    reg.setValue("tree/itemChangedColourUnexpanded", colourToString(itemChangedColours.second));

    reg.setValue("highlighter/colour", colourToString(highlighterColour));
    reg.setValue("highlighter/style", static_cast<int>(highlighterStyle));
    reg.setValue("highlighter/borderThickness", highlighterBorderThickness);

    String infoLabelString = infoLabels.join(",");
    reg.setValue("infoWindow/labels", infoLabelString);

    reg.setValue("logging/enable", enableLogging);
    reg.setValue("logging/outputFolder", logOutputFolder);

    reg.setValue("applicationStyle", appStyle);
    reg.setValue("allowInspectOwnWindows", allowInspectOwnWindows);
}
