/////////////////////////////////////////////////////////////////////
// File: Resources.cpp                                             //
// Date: 29/7/10                                                   //
// Desc: Manages loading and accessing resources such as window    //
//   class definitions, style definitions, and icons.              //
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

#include "inspector.h"
#include "window_detective/Logger.h"
#include "window_detective/main.h"
using namespace inspector;


QMap<String,WindowClass*> Resources::windowClasses;
WindowStyleList Resources::allWindowStyles;
WindowStyleList Resources::generalWindowStyles;
WindowClassStyleList Resources::classStyles;
QHash<uint,String> Resources::generalMessageNames;
QMap<String,QMap<uint,String>*> Resources::constants;
QMap<String,QIcon> Resources::windowClassIcons;
QIcon Resources::defaultWindowIcon;

/*------------------------------------------------------------------+
| Loads all resources from the application's directory. If a user   |
| directory is given, it will append any resources defined there.   |
+------------------------------------------------------------------*/
void Resources::load(String appDir, String userDir) {
    // Load the icon for windows that don't have an icon.
    defaultWindowIcon = QIcon(appDir + "/window_class_icons/generic_window.png");
    if (defaultWindowIcon.isNull()) {
        Logger::debug("Default window icon (generic_window.png) not found");
    }

    // Clear all lists since loadXXX will append to them
    windowClasses = QMap<String,WindowClass*>();
    allWindowStyles = WindowStyleList();
    generalWindowStyles = WindowStyleList();
    classStyles = WindowClassStyleList();
    generalMessageNames = QHash<uint,String>();
    constants = QMap<String,QMap<uint,String>*>();
    IniFile ini;

    // Load from application directory
    ini = IniFile(appDir + "/system_classes.ini");
    loadSystemClasses(ini);
    ini = IniFile(appDir + "/window_styles.ini");
    loadWindowStyles(ini);
    ini = IniFile(appDir + "/window_messages.ini");
    loadWindowMessages(ini);
    ini = IniFile(appDir + "/constants.ini");
    loadConstants(ini);

    // Load from user directory, if it exists
    if (userDir.isEmpty() || !QDir(userDir).exists())
        return;
    ini = IniFile(userDir + "/window_styles.ini");
    loadWindowStyles(ini);
    ini = IniFile(userDir + "/window_messages.ini");
    loadWindowMessages(ini);
}

/*------------------------------------------------------------------+
| Load the list of known Win32 window classes. These are all the    |
| basic controls such as Static and Button.                         |
+------------------------------------------------------------------*/
void Resources::loadSystemClasses(IniFile &ini) {
    while (!ini.isAtEnd()) {
        QStringList values = ini.parseLine();
        if (values.size() == 2) {
            String name = values.at(0);
            String friendlyName = values.at(1);
            windowClasses.insert(name, new WindowClass(name, friendlyName));
        }
        else {
            Logger::error("Could not parse line in \"" +
                        ini.getFileName() + "\":\n" +
                        ini.getCurrentLine());
        }
        ini.selectNextEntry();
    }
}

/*------------------------------------------------------------------+
| Load the list of window style definitions.                        |
+------------------------------------------------------------------*/
void Resources::loadWindowStyles(IniFile &ini) {
    WindowStyle* newStyle = NULL;

    while (!ini.isAtEnd()) {
        QStringList classNames = ini.currentGroup().split(',');
        QStringList values = ini.parseLine();

        if (values.size() == 5 || values.size() == 6) {
            if (classNames.first() == "all") {
                newStyle = new WindowStyle(true);
                allWindowStyles.append(newStyle);
                generalWindowStyles.append(newStyle);
            }
            else {
                newStyle = new WindowStyle(false);
                allWindowStyles.append(newStyle);

                // Add this style to each class's list of applicable styles
                WindowClass* wndClass;
                for (int i = 0; i < classNames.size(); i++) {
                    wndClass = windowClasses[classNames[i]];
                    wndClass->addApplicableStyle(newStyle);
                }
            }
            newStyle->readFrom(values);
        }
        else {
            Logger::error("Could not parse line in \"" +
                        ini.getFileName() + "\":\n" +
                        ini.getCurrentLine());
        }
        ini.selectNextEntry();
    }
}

/*------------------------------------------------------------------+
| Load the list of names of each window message.                    |
+------------------------------------------------------------------*/
void Resources::loadWindowMessages(IniFile &ini) {
    while (!ini.isAtEnd()) {
        bool ok;
        QStringList classNames = ini.currentGroup().split(',');
        QStringList values = ini.parseLine();

        if (values.size() == 2) {
            uint id = values.at(0).toUInt(&ok, 0);
            String name = values.at(1);
            if (classNames.first() == "all") {
                generalMessageNames.insert(id, name);
            }
            else {
                // Make sure it's not already used
                if (generalMessageNames.contains(id)) {
                    Logger::warning("Message id " + hexString(id) +
                                " defined for class " + ini.currentGroup() +
                                " is already defined as a general window message");
                }

                // Add this message to each class's list
                WindowClass* wndClass;
                for (int i = 0; i < classNames.size(); i++) {
                    wndClass = windowClasses[classNames[i]];
                    wndClass->addApplicableMessage(id, name);
                }
            }
        }
        else {
            Logger::error("Could not parse line in \"" +
                        ini.getFileName() + "\":\n" +
                        ini.getCurrentLine());
        }
        ini.selectNextEntry();
    }
}

/*------------------------------------------------------------------+
| Load the list of system defined constants.                        |
+------------------------------------------------------------------*/
void Resources::loadConstants(IniFile &ini) {
    while (!ini.isAtEnd()) {
        bool ok;
        String enumName = ini.currentGroup();
        QStringList values = ini.parseLine();

        if (values.size() == 2) {
            uint id = values.at(0).toULong(&ok, 0);
            String name = values.at(1);

            // Add the enum group if it does not already exist
            if (!constants.contains(enumName)) {
                constants.insert(enumName, new QMap<uint,String>());
            }
            // Now add the constant to the enum group
            constants.value(enumName)->insert(id, name);
        }
        else {
            Logger::error("Could not parse line in \"" +
                        ini.getFileName() + "\":\n" +
                        ini.getCurrentLine());
        }

        ini.selectNextEntry();
    }
}

/*------------------------------------------------------------------+
| Checks if the given constant id exists in the given enum.         |
+------------------------------------------------------------------*/
bool Resources::hasConstant(String enumName, uint id) {
    if (!constants.contains(enumName))
        return false;
    return constants.value(enumName)->contains(id);
}

/*------------------------------------------------------------------+
| Returns the name of the given constant id in the given enum.      |
| If the constant does not exist, a string representation of the id |
| will be returned.                                                 |
+------------------------------------------------------------------*/
String Resources::getConstantName(String enumName, uint id) {
    if (!hasConstant(enumName, id))
        return String::number(id);
    return constants.value(enumName)->value(id);
}

/*------------------------------------------------------------------+
| Returns the icon representing the given window class. If it's not |
| already loaded, the app data then user data folders               |
| (in data/window_class_icons) are searched to find a png or ico    |
| file of the given name.                                           |
+------------------------------------------------------------------*/
QIcon Resources::getWindowClassIcon(String name) {
    // If it's cached, return that
    if (windowClassIcons.contains(name)) {
        return windowClassIcons.value(name);
    }

    // First try app data, png format
    String path = appPath()+"/data/window_class_icons/"+name+".png";
    if (!QFile(path).exists()) {
        // Check user data, png format
        path = userPath()+"/data/window_class_icons/"+name+".png";
        if (!QFile(path).exists()) {
            // Still doesn't exist?, try ico format
            path = userPath()+"/data/window_class_icons/"+name+".ico";
            if (!QFile(path).exists()) {
                // If it doesn't exist at all, use default
                return defaultWindowIcon;
            }
        }
    }
    windowClassIcons.insert(name, QIcon(path));
    return windowClassIcons.value(name);
}

WindowStyleList Resources::getStandardWindowStyles() {
    WindowStyleList standardStyles;
    WindowStyleList::const_iterator i;

    for (i = allWindowStyles.constBegin(); i != allWindowStyles.constEnd(); i++) {
        if (!(*i)->isExtended()) standardStyles.append(*i);
    }
    return standardStyles;
}

WindowStyleList Resources::getExtendedWindowStyles() {
    WindowStyleList extendedStyles;
    WindowStyleList::const_iterator i;

    for (i = allWindowStyles.constBegin(); i != allWindowStyles.constEnd(); i++) {
        if ((*i)->isExtended()) extendedStyles.append(*i);
    }
    return extendedStyles;
}
