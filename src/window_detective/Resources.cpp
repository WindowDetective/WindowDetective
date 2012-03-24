//////////////////////////////////////////////////////////////////////////
// File: Resources.cpp                                                  //
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

#include "inspector/inspector.h"
#include "window_detective/Logger.h"
#include "window_detective/main.h"
#include "window_detective/StringFormatter.h"


QHash<String,WindowClass*> Resources::windowClasses;
WindowStyleList Resources::allWindowStyles;
WindowStyleList Resources::generalWindowStyles;
WindowClassStyleList Resources::classStyles;
QHash<uint,String> Resources::generalMessageNames;
QHash<String,QHash<uint,String>*> Resources::classMessageNames;
QHash<String,StructDefinitionPair*> Resources::messageStructDefns;
QHash<String,QHash<uint,String>*> Resources::constants;
QHash<String,QIcon> Resources::windowClassIcons;
QIcon Resources::defaultWindowIcon;

/*--------------------------------------------------------------------------+
| Loads all resources from the application's directory. If a user           |
| directory is given, it will append any resources defined there.           |
+--------------------------------------------------------------------------*/
void Resources::load(String appDir, String userDir) {
    // Load the icon for windows that don't have an icon.
    defaultWindowIcon = QIcon(appDir + "\\window_class_icons\\generic_window.png");
    if (defaultWindowIcon.isNull()) {
        Logger::debug("Default window icon (generic_window.png) not found");
    }

    IniFile ini;

    // Create message struct definitions TODO: load from xml
    createStructDefinitions(messageStructDefns);

    // Load from application directory
    loadSystemClasses(IniFile(appDir + "\\system_classes.ini"));
    loadConstants(IniFile(appDir + "\\constants.ini"));
    loadWindowStyles(IniFile(appDir + "\\window_styles.ini"));
    loadWindowMessages(IniFile(appDir + "\\window_messages.ini"));

    // Load from user directory, if it exists
    if (!userDir.isEmpty() && QDir(userDir).exists()) {
        loadWindowMessages(IniFile(userDir + "\\window_messages.ini"));
    }
}

/*--------------------------------------------------------------------------+
| Load the list of known Win32 window classes. These are all the            |
| basic controls such as Static and Button.                                 |
+--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------+
| Load the list of window style definitions.                                |
+--------------------------------------------------------------------------*/
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

/*--------------------------------------------------------------------------+
| Load the list of names of each window message.                            |
| For messages applicable to a certain window class, the class's name is    |
| used instead of a WindowClass object, because those objects are created   |
| on demand (i.e. when we actually see a window of that class).             |
+--------------------------------------------------------------------------*/
void Resources::loadWindowMessages(IniFile &ini) {
    while (!ini.isAtEnd()) {
        bool ok;
        QStringList classNames = ini.currentGroup().split(',');
        QStringList values = ini.parseLine();

        if (values.size() == 2) {
            uint msgId = values.at(0).toUInt(&ok, 0);
            String msgName = values.at(1);
            if (classNames.first() == "all") {
                generalMessageNames.insert(msgId, msgName);
            }
            else {
                // Make sure it's not already used
                if (generalMessageNames.contains(msgId)) {
                    Logger::warning("Message id " + hexString(msgId) +
                                " defined for class " + ini.currentGroup() +
                                " is already defined as a general window message");
                }

                // Add this message for each applicable window class
                foreach (String className, classNames) {
                    if (!classMessageNames.contains(className)) {
                        classMessageNames.insert(className, new QHash<uint,String>());
                    }
                    classMessageNames.value(className)->insert(msgId, msgName);
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

/*--------------------------------------------------------------------------+
| Load the list of system defined constants.                                |
+--------------------------------------------------------------------------*/
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
                constants.insert(enumName, new QHash<uint,String>());
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

/*--------------------------------------------------------------------------+
| Checks if the given constant id exists in the given enum.                 |
+--------------------------------------------------------------------------*/
bool Resources::hasConstant(String enumName, uint id) {
    QHash<uint,String>* map = constants.value(enumName, NULL);
    return map ? map->contains(id) : false;
}

/*--------------------------------------------------------------------------+
| Returns the name of the given constant id in the given enum.              |
| If the constant does not exist, a string representation of the id         |
| will be returned.                                                         |
+--------------------------------------------------------------------------*/
String Resources::getConstant(String enumName, uint id) {
    if (!hasConstant(enumName, id))
        return String::number(id);
    return constants.value(enumName)->value(id);
}

/*--------------------------------------------------------------------------+
| Returns all constants in the given enum.                                  |
+--------------------------------------------------------------------------*/
QHash<uint,String> Resources::getConstants(String enumName) {
    QHash<uint,String>* map = constants.value(enumName, NULL);
    return map ? *map : QHash<uint, String>();
}

/*--------------------------------------------------------------------------+
| Returns all window messages applicable to the given window class.         |
| The name is used as the key, rather than a WindowClass object, because    |
| such an object may not yet exist for the given class.                     |
+--------------------------------------------------------------------------*/
QHash<uint,String> Resources::getWindowClassMessages(String windowClassName) {
    QHash<uint,String>* map = classMessageNames.value(windowClassName, NULL);
    return map ? *map : QHash<uint, String>();
}

/*--------------------------------------------------------------------------+
| Returns the icon representing the given window class. If it's not         |
| already loaded, the data folders (data/window_class_icons) are searched   |
| to find a png or ico file of the given name.                              |
| User data is checked first, so users are able to override app icons.      |
+--------------------------------------------------------------------------*/
QIcon Resources::getWindowClassIcon(String name) {
    // If it's cached, return that
    QHash<QString, QIcon>::const_iterator i = windowClassIcons.find(name);
    if (i != windowClassIcons.end()) {
        return i.value();
    }

    // First try user data - png format
    String path = userPath()+"/data/window_class_icons/"+name+".png";
    if (!QFile(path).exists()) {
        // Doesn't exist? Try ico format
        path = userPath()+"/data/window_class_icons/"+name+".ico";
        if (!QFile(path).exists()) {
            // If no user file, check app data - png format
            path = appPath()+"/data/window_class_icons/"+name+".png";
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

    for (i = allWindowStyles.begin(); i != allWindowStyles.end(); i++) {
        if (!(*i)->isExtended()) standardStyles.append(*i);
    }
    return standardStyles;
}

WindowStyleList Resources::getExtendedWindowStyles() {
    WindowStyleList extendedStyles;
    WindowStyleList::const_iterator i;

    for (i = allWindowStyles.begin(); i != allWindowStyles.end(); i++) {
        if ((*i)->isExtended()) extendedStyles.append(*i);
    }
    return extendedStyles;
}
