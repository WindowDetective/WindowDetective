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

    // Find and load icon. Can be either PNG or ICO file
    icon = Resources::getWindowClassIcon(name);
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
    icon = Resources::getWindowClassIcon(name);
}

/*------------------------------------------------------------------+
| Updates properties of this window class from the given struct     |
+------------------------------------------------------------------*/
void WindowClass::updateInfoFrom(WindowInfoStruct* info) {
    classExtraBytes = info->wndClassInfo.cbClsExtra;
    windowExtraBytes = info->wndClassInfo.cbWndExtra;
    if (backgroundBrush) delete backgroundBrush;  // Remove old one
    backgroundBrush = new WinBrush(info->wndClassInfo.hbrBackground, info->logBrush);
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