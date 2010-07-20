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
//   "smart settings". This means that they are not remembered     //
//   immediately, but are only remembered if they are set to that  //
//   value regularly. See comments in SmartValue.h.                //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010 XTAL256

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

#ifndef SETTINGS_H
#define SETTINGS_H

#include "window_detective/include.h"
#include "ui/HighlightWindow.h"
#include "ui/custom_widgets/TreeItem.h"
#include "SmartSettings.h"


class Settings {
private:
    static int appInstalled;
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

    static bool isAppInstalled();
    static void read();
    static void write();
};

#endif   // SETTINGS_H
