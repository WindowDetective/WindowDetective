/////////////////////////////////////////////////////////////////////
// File: WindowMisc.cpp                                            //
// Date: 4/3/10                                                    //
// Desc: Definitions of all window related classes. Most of these  //
//   objects are not often changed (unlike Window), and as such    //
//   they are only updated once in the contructor.                 //
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

#include "inspector.h"
#include "WindowManager.h"
#include "MessageHandler.h"
#include "window_detective/Logger.h"
#include "RemoteFunctions.h"
using namespace inspector;


/***********************/
/*** Resources class ***/
/***********************/

QMap<String,WindowClass*> Resources::windowClasses;
WindowStyleList Resources::allWindowStyles;
WindowStyleList Resources::generalWindowStyles;
WindowClassStyleList Resources::classStyles;
QHash<uint,String> Resources::messageNames;
QMap<String,QMap<uint,String>*> Resources::constants;

/*------------------------------------------------------------------+
| Loads all resources from the application's directory. If a user   |
| directory is given, it will append any resources defined there.   |
+------------------------------------------------------------------*/
void Resources::load(String appDir, String userDir) {
    // Clear all lists since loadFromDir will append to them
    windowClasses = QMap<String,WindowClass*>();
    allWindowStyles = WindowStyleList();
    generalWindowStyles = WindowStyleList();
    classStyles = WindowClassStyleList();
    messageNames = QHash<uint,String>();
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
    if (userDir.isEmpty() || !QFile(userDir).exists())
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
        QStringList values = ini.readLine();
        String name = values.at(0);
        String displayName = values.at(1);
        windowClasses.insert(name, new WindowClass(name, displayName));
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
        QStringList values = ini.readLine();

        if (classNames.first() == "all") {
            newStyle = new WindowStyle(true);
            allWindowStyles.append(newStyle);
            generalWindowStyles.append(newStyle);
        }
        else {
            newStyle = new WindowStyle(false);
            allWindowStyles.append(newStyle);
            WindowClass* wndClass;

            // Add this style to each class's list applicable styles
            for (int i = 0; i < classNames.size(); i++) {
                wndClass = windowClasses[classNames[i]];
                wndClass->addApplicableStyle(newStyle);
            }
        }
        newStyle->readFrom(values);
        ini.selectNextEntry();
    }
}

/*------------------------------------------------------------------+
| Load the list of names of each window message.                    |
+------------------------------------------------------------------*/
void Resources::loadWindowMessages(IniFile &ini) {
    while (!ini.isAtEnd()) {
        bool ok;
        QStringList values = ini.readLine();
        uint id = values.at(0).toUInt(&ok, 0);
        String name = values.at(1);
        messageNames.insert(id, name);
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
        QStringList values = ini.readLine();

        uint id = values.at(0).toULong(&ok, 0);
        String name = values.at(1);

        // Add the enum group if it does not already exist
        if (!constants.contains(enumName)) {
            constants.insert(enumName, new QMap<uint,String>());
        }
        // Now add the constant to the enum group
        constants.value(enumName)->insert(id, name);

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


/*************************/
/*** WindowStyle class ***/
/*************************/

/*------------------------------------------------------------------+
| WindowStyle Constructor                                           |
+------------------------------------------------------------------*/
WindowStyle::WindowStyle(bool isGeneric) :
    value(0), isGeneric(isGeneric), extended(false) {
}

/*------------------------------------------------------------------+
| Creates this object from the given string values.                 |
| The values are as follows (same as in INI file):                  |
|   "id, name, depends, excludes, description"                      |
+------------------------------------------------------------------*/
void WindowStyle::readFrom(QStringList values) {
    bool ok;

    value = values.at(0).toULong(&ok, 0);
    name = values.at(1);
    extended = !values.at(2).compare("true",Qt::CaseInsensitive);
    depends = values.at(3).toULong(&ok, 0);
    excludes = values.at(4).toULong(&ok, 0);
    description = values.at(5);
}

/*------------------------------------------------------------------+
| Returns true if this style can be applied to a window of the      |
| given window class.                                               |
+------------------------------------------------------------------*/
bool WindowStyle::isValidFor(WindowClass* windowClass) {
    // Generic window styles (WS_*) are valid for any class
    if (isGeneric)
        return true;

    // Otherwise, check if it is one of the class's applicable styles
    WindowStyleList list = windowClass->getApplicableWindowStyles();
    WindowStyleList::const_iterator i;
    for (i = list.begin(); i != list.end(); ++i) {
        if (*i == this) return true;
    }
    return false;
}


/******************************/
/*** WindowClassStyle class ***/
/******************************/

/*------------------------------------------------------------------+
| WindowClassStyle Constructor                                      |
+------------------------------------------------------------------*/
WindowClassStyle::WindowClassStyle(String name, uint value, String desc) :
    name(name), value(value), description(desc) {
}


/***************************/
/*** WindowMessage class ***/
/***************************/

WindowMessage::WindowMessage(Window* window, UINT id,
                             WPARAM wParam, LPARAM lParam,
                             LRESULT returnValue) :
    window(window), id(id),
    wParam(wParam), lParam(lParam),
    returnValue(returnValue) {
}

WindowMessage::WindowMessage(HWND hWnd, UINT id,
                             WPARAM wParam, LPARAM lParam,
                             LRESULT returnValue) :
    id(id), wParam(wParam), lParam(lParam),
    returnValue(returnValue) {
    window = WindowManager::current()->find(hWnd);
}

String WindowMessage::nameForId(UINT id) {
    if (Resources::messageNames.contains(id)) {
        return Resources::messageNames.value(id);
    }
    else if (id >= WM_USER && id <= 0xFFFF) {
        return "WM_USER + " + String::number(id-WM_USER);
    }
    else {
        return "<" + TR("unknown: ") + String::number(id) + ">";
    }
}

String WindowMessage::getName() const {
    return WindowMessage::nameForId(this->id);
}

LRESULT WindowMessage::send() {
    if (!window) return 0;
    /* FIXME: I get a link error for some reason (sendMessage unresolved)
    try {
        returnValue = window->sendMessage<LRESULT,WPARAM,LPARAM>(id, wParam, lParam);
    }
    catch (TimeoutError e) {
        Logger::error(e);
        returnValue = 0;
    }*/
    return returnValue;
}


/*************************/
/*** WindowClass class ***/
/*************************/

/*------------------------------------------------------------------+
| WindowClass basic constructor                                     |
| Used for creating a class that an application has registered.     |
+------------------------------------------------------------------*/
WindowClass::WindowClass(String name) :
    name(name), displayName(),
    styles(), applicableWindowStyles(),
    classExtraBytes(0), windowExtraBytes(0),
    backgroundBrush(NULL),
    native(false), creatorInst(NULL) {
    icon = WindowManager::current()->defaultWindowIcon;
}

WindowClass::~WindowClass() {
    if (backgroundBrush) delete backgroundBrush;
}

/*------------------------------------------------------------------+
| WindowClass full constructor                                      |
| Used for creating a standard Win32 class from the INI file.       |
+------------------------------------------------------------------*/
WindowClass::WindowClass(String name, String displayName, bool isNative) :
    name(name), displayName(displayName),
    styles(), applicableWindowStyles(),
    classExtraBytes(0), windowExtraBytes(0),
    backgroundBrush(NULL),
    native(isNative) {

    // If this is a system class (e.g. Button), it will not have a HINSTANCE
    if (native) creatorInst = NULL;

    // Find and load icon. Can be either PNG or ICO file
    icon = QIcon("data/window_class_icons/" + name + ".png");
}

/*------------------------------------------------------------------+
| Updates properties of this window class. Needs an instance of a   |
| window in order to get certain class info.                        |
| Note: GetClassInfo can only be called from a remote thread so,    |
| for efficiency, this method should only be called when needed.    |
+------------------------------------------------------------------*/
bool WindowClass::updateClassInfo(HWND instance) {
    WNDCLASSEX classInfo;
    classInfo.cbSize = sizeof(WNDCLASSEX);
    WCHAR* szName = (WCHAR*)name.utf16();
    DWORD result = GetWindowClassInfoRemote(szName, instance, &classInfo);
    if (result != ERROR_SUCCESS) {
        String errorStr = TR("Could not get class info for ")+getDisplayName();
        if (result == -1) {   // unknown error occurred
            Logger::warning(errorStr);
        }
        else {
            Logger::osWarning(result, errorStr);
        }
        return false;
    }

    classExtraBytes = classInfo.cbClsExtra;
    windowExtraBytes = classInfo.cbWndExtra;
    backgroundBrush = new WinBrush(classInfo.hbrBackground);

    return true;
}

/*------------------------------------------------------------------+
| Some built-in Win32 classes do not have a very descriptive name   |
| so return the class name followed by a "friendly" name.           |
| e.g. "msctls_statusbar32 (Status Bar)"                            |
|      "#32769 (Desktop)"                                           |
+------------------------------------------------------------------*/
String WindowClass::getDisplayName() {
    if (displayName.isEmpty())
        return name;
    else
        return name + " (" + displayName + ")";
}


/**********************/
/*** WinBrush class ***/
/**********************/

/*------------------------------------------------------------------+
| WinBrush constructor                                              |
| Gets the brush data from the handle by calling GetObject.         |
+------------------------------------------------------------------*/
WinBrush::WinBrush(HBRUSH handle) :
    handle(handle) {
    /* TODO: Cannot GetObject if the handle belongs to another process.
         Maybe for some types (or "shared handles") but not brushes.
         So just pass style, colour and hatch to constructor.
         Then GetObject in injected thread, should also get other stuff too.
    LOGBRUSH brush;

    if (GetObject(handle, sizeof(LOGBRUSH), &brush)) {
        style = brush.lbStyle;
        colour = brush.lbColor;
        hatchType = brush.lbHatch;
    }
    else {
        Logger::osWarning(TR("Unable to get LOGBRUSH data for ") +
                    hexString((uint)handle));
        style = colour = hatchType = 0;
    }*/
}

/*String WinBrush::getStyleName() {
    return Resources::getConstantName("BrushStyles", style);
}*/


/**************************/
/*** TimeoutError class ***/
/**************************/

TimeoutError::TimeoutError(const WindowMessage& msg) :
    Error("Timeout Error") {
    QTextStream stream(&message);
    stream << TR("The message ") << msg.getName()
           << TR(" sent to window ")
           << msg.window->displayName()
           << TR(" has timed-out.\n wParam = ")
           << String::number((uint)msg.wParam)
           << TR(", lParam = ")
           << String::number((uint)msg.lParam);
}