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

bool Settings::appInstalled;

bool Settings::use32bitCursor;
bool Settings::canPickTransparentWindows;
bool Settings::hideWhilePicking;
uint Settings::messageTimeoutPeriod;
bool Settings::greyHiddenWindows;
QColor Settings::itemCreatedColour;
QColor Settings::itemDestroyedColour;
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

void Settings::read() {
    // If the registry key does not exist (because this app was not
    // installed), then don't create it. No settings will be saved.
    String regName;
    HKEY key;
    LONG result = RegOpenKey(HKEY_CURRENT_USER, L"Software\\Window Detective", &key);
    if (result != ERROR_SUCCESS) {
        appInstalled = false;
        regName = "";
    }
    else {
        appInstalled = true;
        regName = APP_NAME;
    }

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
    itemCreatedColour = stringToColour(reg.value("tree/itemCreatedColour", "0,255,0").toString());
    itemDestroyedColour = stringToColour(reg.value("tree/itemDestroyedColour", "255,0,0").toString());

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
    if (!appInstalled)
        return;

    QSettings reg;
    reg.setValue("use32bitCursor", use32bitCursor);
    reg.setValue("canPickTransparentWindows", canPickTransparentWindows);
    reg.setValue("hideWhilePicking", hideWhilePicking);
    reg.setValue("messageTimeoutPeriod", messageTimeoutPeriod);
    reg.setValue("regexType", static_cast<int>(regexType));

    reg.setValue("tree/greyHiddenWindows", greyHiddenWindows);
    reg.setValue("tree/changeDuration", treeChangeDuration);
    reg.setValue("tree/itemCreatedColour", colourToString(itemCreatedColour));
    reg.setValue("tree/itemDestroyedColour", colourToString(itemDestroyedColour));

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

/*------------------------------------------------------------------+
 | Parse the given string to a colour in the form                   |
 | "red,green,blue[,alpha]". If alpha is omitted, it's set to 255.  |
 | Returns the default colour red on error.                         |
 +------------------------------------------------------------------*/
QColor Settings::stringToColour(String string) {
    QColor colour;
    QStringList rgbList = string.split(",");
    if (rgbList.size() != 3 && rgbList.size() != 4)
        goto error;     // goto! :O
    bool isOk;
    colour.setRed(rgbList[0].toInt(&isOk));
    if (!isOk) goto error;
    colour.setGreen(rgbList[1].toInt(&isOk));
    if (!isOk) goto error;
    colour.setBlue(rgbList[2].toInt(&isOk));
    if (!isOk) goto error;
    if (rgbList.size() == 4) {
        colour.setAlpha(rgbList[3].toInt(&isOk));
        if (!isOk) goto error;
    }
    else {
        colour.setAlpha(255);
    }
    return colour;          // If everything went ok

    error:
    Logger::error(TR("Could not read colour from settings: ") + string +
                  TR(". Colour must be in the form \"red,green,blue[,alpha]\"."));
    return QColor(255, 0, 0);  // Return default colour on error
}

/*------------------------------------------------------------------+
 | Return a string representation of the colour in the form         |
 | "red,green,blue[,alpha]". If alpha is 255, it is omitted.        |
 +------------------------------------------------------------------*/
String Settings::colourToString(QColor colour) {
    String s = String::number(colour.red()) + "," +
               String::number(colour.green()) + "," +
               String::number(colour.blue());
    if (colour.alpha() != 255)
        s += "," + String::number(colour.alpha());
    return s;
}

/*------------------------------------------------------------------+
 | Returns an integer indicating the version of the operating       |
 | system this application is running on. XP is 501.                |
 +------------------------------------------------------------------*/
int Settings::getOSVersion() {
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&info);
    return (info.dwMajorVersion * 100) + info.dwMinorVersion;
}
