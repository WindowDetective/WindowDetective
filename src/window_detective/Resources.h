//////////////////////////////////////////////////////////////////////////
// File: Resources.h                                                    //
// Date: 29/7/10                                                        //
// Desc: Manages loading and accessing resources such as window         //
//   class definitions, style definitions, and icons.                   //
//////////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2012 XTAL256

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

#ifndef RESOURCES_H
#define RESOURCES_H

#include "window_detective/Error.h"
#include "window_detective/IniFile.h"
#include "inspector/DynamicStruct.h"
#include "inspector/MessageStructDefinitions.h"


class Resources {
public:
    static QHash<String,WindowClass*> windowClasses;
    static WindowStyleList allWindowStyles;
    static WindowStyleList generalWindowStyles;
    static WindowClassStyleList classStyles;
    static QHash<uint,String> generalMessageNames;
    static QHash<String,QHash<uint,String>*> classMessageNames;
    static QHash<String,StructDefinitionPair*> messageStructDefns;
    static QHash<String,QHash<uint,String>*> constants;
    static QHash<String,QIcon> windowClassIcons;
    static QIcon defaultWindowIcon;

    static void load(String appDir, String userDir = String());
    static void loadSystemClasses(IniFile &ini);
    static void loadWindowStyles(IniFile &ini);
    static void loadWindowMessages(IniFile &ini);
    static void loadConstants(IniFile &ini);

    static bool hasConstant(String enumName, uint id);
    static String getConstant(String enumName, uint id);
    static QHash<uint,String> getConstants(String enumName);
    static QHash<uint,String> getWindowClassMessages(String windowClassName);
    static QIcon getWindowClassIcon(String name);
    static WindowStyleList getStandardWindowStyles();
    static WindowStyleList getExtendedWindowStyles();
};


#endif   // RESOURCES_H