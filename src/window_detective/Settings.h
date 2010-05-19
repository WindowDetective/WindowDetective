/////////////////////////////////////////////////////////////////////
// File: Settings.h                                                //
// Date: 26/2/10                                                   //
// Desc: Handles reading and writing settings that are stored in   //
//   the Windows registry. Values are written when the application //
//   exits or when the user changes them in the preferences.       //
//   If the application is run on a machine which it is not        //
//   installed on, no settings are saved to the registry unless    //
//   specified by the user.                                        //
//   Certain values such as window position are stored as          //
//   "soft settings". This means that they are not remembered      //
//   immediately, but are only remembered if they are set to that  //
//   value regularly. See comments on SoftValue class below.       //
/////////////////////////////////////////////////////////////////////

#ifndef SETTINGS_H
#define SETTINGS_H

#include "window_detective/include.h"
#include "ui/HighlightWindow.h"
#include "ui/custom_widgets/TreeItem.h"

class Settings {
private:
    static bool appInstalled;
public:
    static bool use32bitCursor;
    static bool canPickTransparentWindows;
    static bool hideWhilePicking;
    static uint messageTimeoutPeriod;
    static bool greyHiddenWindows;
    static QPair<QColor,QColor> itemCreatedColours;
    static QPair<QColor,QColor> itemDestroyedColours;
    static QPair<QColor,QColor> itemChangedColours;
    static uint treeChangeDuration;
    static QRegExp::PatternSyntax regexType;
    static QColor highlighterColour;
    static HighlightStyle highlighterStyle;
    static int highlighterBorderThickness;
    static QStringList infoLabels;
    static bool enableLogging;
    static String logOutputFolder;
    static String appStyle;
    static bool allowInspectOwnWindows;  // Undocumented option

    static void read();
    static void write();
    static bool isAppInstalled() { return appInstalled; }
};


/*------------------------------------------------------------------+
 | This class stores a "soft" value, a setting which is remembered  |
 | based on the number of times the value is set. It contains a     |
 | current value, as well as a list of previously set values.       |
 | The list contains a fixed number of values and acts like a       |
 | queue. When the current value is changed, it is also pushed      |
 | onto the list and the last one is removed.                       |
 | The value that is remembered (and becomes the current) is the    |
 | one which occurs the most frequently in the list. If all values  |
 | are different, then the current value is kept.                   |
 | This means that the value will only be remembered if the user    |
 | frequently sets it to a specific value. If they only change it   |
 | once, then it will not remember it and will keep the value it    |
 | had always been. It also can have a threshold which ensures      |
 | that the values don't have to be all identical but can be        |
 | within the threshold.                                            |
 +------------------------------------------------------------------*/
template <class T>
class SoftValue {
private:
    T currentValue;
    T* previousValues;
    int numValues;
    T threshold;

public:
    SoftValue();
    ~SoftValue();

    // operator= for read and write
};

#endif   // SETTINGS_H