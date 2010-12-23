/////////////////////////////////////////////////////////////////////
// File: Window.cpp                                                //
// Date: 20/2/10                                                   //
// Desc: Creates window objects from a real Windows handle (HWND)  //
//   as well as other window related objects such as window        //
//   classes, styles and messages.                                 //
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
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
#include "inspector/RemoteFunctions.h"
using namespace inspector;

HighlightWindow Window::flashHighlighter;

/*------------------------------------------------------------------+
| Constructor                                                       |
| Creates a Window object from the real window handle               |
+------------------------------------------------------------------*/
Window::Window(HWND handle) :
    handle(handle),
    windowClass(NULL),
    parent(NULL),
    styles(), exStyles(),
    styleBits(0), exStyleBits(0),
    props(), font(NULL),
    process(NULL), threadId(0) {
}

/*------------------------------------------------------------------+
| Copy Constructor                                                  |
+------------------------------------------------------------------*/
Window::Window(const Window& other) :
    handle(other.handle),
    windowClass(other.windowClass),
    parent(other.parent),
    styles(other.styles),
    exStyles(other.exStyles),
    styleBits(other.styleBits),
    exStyleBits(other.exStyleBits),
    props(other.props),
    font(NULL),
    process(other.process),
    threadId(other.threadId) {
    if (other.font) font = new WinFont(*other.font);
}

/*------------------------------------------------------------------+
| Destructor                                                        |
+------------------------------------------------------------------*/
Window::~Window() {
    if (font) delete font;
}

/*------------------------------------------------------------------+
| Send a message to the window with the given parameters            |
+------------------------------------------------------------------*/
template <class ReturnType, class FirstType, class SecondType>
ReturnType Window::sendMessage(UINT msg, FirstType wParam, SecondType lParam) {
    DWORD result;
    LRESULT returnValue;

    returnValue = SendMessageTimeout(this->handle, msg,
                (WPARAM)wParam, (LPARAM)lParam, SMTO_ABORTIFHUNG,
                Settings::messageTimeoutPeriod, &result);
    if (!returnValue) {
        DWORD error = GetLastError();
        if (error == ERROR_TIMEOUT) {
            throw TimeoutError(WindowMessage(this, msg,
                        (WPARAM)wParam, (LPARAM)lParam));
        }
    }
    return (ReturnType)result;
}

/*------------------------------------------------------------------+
| Send a message to the window with no parameters. Mostly used      |
| for getting a value from the window or for sending an action      |
| message (e.g. WM_CLOSE).                                          |
+------------------------------------------------------------------*/
template <class ReturnType>
ReturnType Window::sendMessage(UINT msg) {
    return sendMessage<ReturnType,int,int>(msg, NULL, NULL);
}

/*------------------------------------------------------------------+
| If this is a child window, then it's coords are relative to it's  |
| parent. If not then they are the same as it's absolute coords.    |
+------------------------------------------------------------------*/
QRect Window::getRelativeDimensions() {
    if (isChild()) {
        RECT rect = RECTFromQRect(windowRect);
        MapWindowPoints(NULL, GetParent(handle), (POINT*)&rect, 2);
        return QRectFromRECT(rect);
    }
    else {
        return windowRect;
    }
}

QPoint Window::getRelativePosition() {
    return getRelativeDimensions().topLeft();
}

/*------------------------------------------------------------------+
| Returns all windows who's parent is this. Note that although this |
| window has a reference to it's parent, the list of children is    |
| built each time this function is called (i.e. it's not cached).   |
+------------------------------------------------------------------*/
WindowList Window::getChildren() {
    return WindowManager::current()->findChildren(this);
}

/*------------------------------------------------------------------+
| Returns all windows who's ancestor is this.                       |
+------------------------------------------------------------------*/
WindowList Window::getDescendants() {
    WindowList allChildren;
    WindowList children = getChildren();
    WindowList::const_iterator i;

    for (i = children.constBegin(); i != children.constEnd(); i++) {
        allChildren.append(*i);
        allChildren.append((*i)->getDescendants());
    }
    return allChildren;
}

/*------------------------------------------------------------------+
| Returns a string suitable for display in the UI.                  |
+------------------------------------------------------------------*/
String Window::getDisplayName() {
    if (!windowClass) {
        // This should only be the case if we haven't updated the data yet
        return hexString((uint)handle);
    }
    else {
        return windowClass->getName()+" ("+hexString((uint)handle)+")";
    }
}


/**********************/
/*** Setter methods ***/
/**********************/

void Window::setText(String newText) {
    const ushort* charData = newText.utf16();
    LRESULT result = sendMessage<LRESULT,int,const ushort*>(WM_SETTEXT, NULL, charData);
    if (!result && !GetLastError()) {
        Logger::osError(TR("Unable to set window text: %1").arg(newText));
    }
}

void Window::setDimensions(QRect rect) {
    SetWindowPos(handle, NULL, rect.x(), rect.y(), rect.width(), rect.height(),
            SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER);
}

void Window::setPosition(QPoint pos) {
    SetWindowPos(handle, NULL, pos.x(), pos.y(), 0, 0,
            SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOSIZE);
}

void Window::setSize(QSize size) {
    SetWindowPos(handle, NULL, 0, 0, size.width(), size.height(),
            SWP_ASYNCWINDOWPOS | SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOMOVE);
}

void Window::setStyleBits(uint styleBits, uint exStyleBits) {
    if (!SetWindowLongPtr(handle, GWL_STYLE, styleBits)) {
        Logger::osError(TR("Unable to set window style"));
    }
    if (!SetWindowLongPtr(handle, GWL_EXSTYLE, exStyleBits)) {
        Logger::osError(TR("Unable to set window extended style"));
    }

    // MSDN: If you have changed certain window data using SetWindowLong,
    //   you must call SetWindowPos for the changes to take effect.
    SetWindowPos(handle, NULL, 0, 0, 0, 0,
            SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
            SWP_NOACTIVATE | SWP_FRAMECHANGED);
}

void Window::setOnTop(bool isOnTop) {
    HWND insertAfter = (isOnTop ? HWND_TOPMOST : HWND_NOTOPMOST);
    SetWindowPos(handle, insertAfter, 0, 0, 0, 0,
            SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);

}


/**********************/
/*** Update methods ***/
/**********************/

void Window::fireUpdateEvent(UpdateReason reason) {
    emit updated(reason);
}

/*------------------------------------------------------------------+
| Updates common properties such as title, size, position and       |
| window class by requesting it from the real window.               |
+------------------------------------------------------------------*/
void Window::update() {
    unicode = IsWindowUnicode(handle);
    updateText();
    updateWindowClass();
    updateWindowInfo();
    updateIcon();
}

/*------------------------------------------------------------------+
| Updates the window title or control text by sending the           |
| WM_GETTEXT message to the window.                                 |
+------------------------------------------------------------------*/
void Window::updateText() {
    WCHAR* charData = NULL;

    try {
        UINT length = sendMessage<UINT>(WM_GETTEXTLENGTH);
        if (length == 0) {
            text = "";
            return;
        }
        charData = new WCHAR[length+1];   // Text length + null terminator
        LRESULT result = sendMessage<LRESULT,UINT,WCHAR*>(WM_GETTEXT, length+1, charData);
        if (result) {
            text = String::fromWCharArray(charData, length);
        }
        else {
            text = "";
        }
    }
    catch (TimeoutError e) {
        Logger::warning(e);
        // TODO: Perhaps do something about it, like try again later
    }

    if (charData) delete[] charData;
}

/*------------------------------------------------------------------+
| Updates the window's class and adds it to the WindowManager's     |
| list if it does not already exist.                                |
+------------------------------------------------------------------*/
void Window::updateWindowClass() {
    WCHAR* charData = new WCHAR[256];
    String className;

    // If it fails, try with a bigger buffer
    if (!GetClassName(handle, charData, 256)) {
        delete[] charData;
        charData = new WCHAR[1024];

        // If it still fails, there's nothing we can do about it
        if (!GetClassName(handle, charData, 1024)) {
            Logger::osWarning(TR("Could not get class name for window %1")
                        .arg(hexString((uint)handle)));
            className = TR("<unknown>");
        }
        else {
            className = String::fromWCharArray(charData);
        }
    }
    else {
        className = String::fromWCharArray(charData);
    }
    delete[] charData;

    // Find existing class or add it as a new one
    if (Resources::windowClasses.contains(className)) {
        windowClass = Resources::windowClasses[className];
    }
    else {
        windowClass = new WindowClass(className);
        Resources::windowClasses.insert(className, windowClass);
    }
}

/*------------------------------------------------------------------+
| Updates various window info including window and client area      |
| rectangle, styles, status and atom type.                          |
+------------------------------------------------------------------*/
void Window::updateWindowInfo() {
    WindowManager* manager = WindowManager::current();
    WINDOWINFO info;
    info.cbSize = sizeof(WINDOWINFO);

    GetWindowInfo(handle, &info);
    windowRect = QRectFromRECT(info.rcWindow);
    clientRect = QRectFromRECT(info.rcClient);
    styleBits = info.dwStyle;
    exStyleBits = info.dwExStyle;
    styles = manager->parseStyle(this, styleBits, false);
    exStyles = manager->parseStyle(this, exStyleBits, true);
    //dwWindowStatus
    //atomWindowType
    //wCreatorVersion
}

/*------------------------------------------------------------------+
| Updates the window's small and large icon. If no small or large   |
| icon has been set, the icon that represents this class is used.   |
+------------------------------------------------------------------*/
void Window::updateIcon() {
    icon = QIcon();
    HICON smallIcon = NULL;
    HICON largeIcon = NULL;
    ICONINFO iconInfo;

    try {
        // Add small and large icons
        smallIcon = sendMessage<HICON,uint,uint>(WM_GETICON, ICON_SMALL, NULL);
        largeIcon = sendMessage<HICON,uint,uint>(WM_GETICON, ICON_BIG, NULL);

        // Sometimes QPixmap::fromWinHICON fails if it cannot get the icon
        // info. I'm not quite sure of the cause, but just check GetIconInfo
        // here to avoid that problem.
        if (smallIcon) {
            bool result = GetIconInfo(smallIcon, &iconInfo);
            if (result) {
                icon.addPixmap(QPixmap::fromWinHICON(smallIcon));
            }
            else {
                Logger::osWarning(TR("Failed to get small icon for window %1")
                                    .arg(getDisplayName()));
            }
        }
        if (largeIcon) {
            bool result = GetIconInfo(largeIcon, &iconInfo);
            if (result) {
                icon.addPixmap(QPixmap::fromWinHICON(largeIcon));
            }
            else {
                Logger::osWarning(TR("Failed to get large icon for window %1")
                                    .arg(getDisplayName()));
            }
        }
    }
    catch (TimeoutError e) {
        Logger::warning(e);
        // TODO: Perhaps do something about it, like try again later
    }
    if (!smallIcon && !largeIcon)
        icon = windowClass->getIcon();
}

/*------------------------------------------------------------------+
| Updates the list of window properties. These properties are set   |
| by calling the SetProc API function.                              |
| Since these properties are not often used, they will only be      |
| updated when they are needed and not in the update() method.      |
+------------------------------------------------------------------*/
void Window::updateProps() {
    props.clear();
    EnumPropsEx(handle, Window::enumProps, reinterpret_cast<ULONG_PTR>(&props));
}

/*------------------------------------------------------------------+
| Updates other properties of this window and it's class. The API   |
| functions used here can only be called from a remote thread so,   |
| for efficiency, this method should only be called when needed.    |
+------------------------------------------------------------------*/
bool Window::updateExtraInfo() {
    WindowInfoStruct info;
    if (!windowClass) return false;

    WCHAR* className = (WCHAR*)windowClass->getName().utf16();
    info.hInst = (HINSTANCE)GetWindowLong(handle, GWL_HINSTANCE);

    // It appears that obtaining font object does not need to be done
    // in the remote process, it can just be done here
    LOGFONT logFont;
    HFONT hFont = sendMessage<HFONT,int,int>(WM_GETFONT, NULL, NULL);
    GetObject(hFont, sizeof(LOGFONTW), (LPVOID)&logFont);

    DWORD result = GetWindowAndClassInfo(className, handle, &info);
    if (result != S_OK) {
        String errorStr = TR("Could not get extended info for ")+getDisplayName();
        if (result == -1) {   // unknown error occurred
            Logger::warning(errorStr);
        }
        else {
            Logger::osWarning(result, errorStr);
        }
        return false;
    }

    windowClass->updateInfoFrom(&info);
    if (font) delete font;  // Remove old one
    font = new WinFont(hFont, logFont);

    return true;
}


/***********************/
/*** Command methods ***/
/***********************/

/*------------------------------------------------------------------+
| Shows the window in front of all others. If 'activate' is true,   |
| the window will become the active one. If 'stay' is true, the     |
| window will stay on top always.                                   |
+------------------------------------------------------------------*/
void Window::show(bool activate, bool stay) {
    HWND insertPos = stay ? HWND_TOPMOST : HWND_TOP;
    if (activate) {
        BOOL result = SetWindowPos(handle, insertPos, 0, 0, 0, 0,
                        SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
        if (!result) {
            Logger::osError(TR("Unable to show window %1").arg(getDisplayName()));
        }
    }
    else {
        ShowWindow(handle, SW_SHOWNA); // Show not active (possibly maximized)
    }
}

/*------------------------------------------------------------------+
| Hides the window. This will clear it's WS_VISIBLE flag.           |
+------------------------------------------------------------------*/
void Window::hide() {
    ShowWindow(handle, SW_HIDE);
}

void Window::maximise() {
    ShowWindow(handle, SW_SHOWMAXIMIZED);
}

void Window::minimise() {
    ShowWindow(handle, SW_FORCEMINIMIZE);
}

/*------------------------------------------------------------------+
| Closes the window by sending it a WM_CLOSE message. The owner     |
| application may do other processing, such as prompting the user   |
| for confirmation, prior to destroying the window.                 |
| If an application processes this message, it should return zero.  |
+------------------------------------------------------------------*/
void Window::close() {
    try {
        LRESULT result = sendMessage<LRESULT,int,int>(WM_CLOSE, NULL, NULL);
        if (result != 0) {
            Logger::info(TR("Window (%1) returned from WM_CLOSE with value %2")
                         .arg(getDisplayName(), String::number(result)));
        }
    }
    catch (TimeoutError e) {
        Logger::error(e);
    }
}

void Window::destroy() {
    // TODO: Use DestroyWindow. Must be called from remote thread
}

/*------------------------------------------------------------------+
| Flashes the highlighter on the window.                            |
+------------------------------------------------------------------*/
void Window::flash() {
    Window::flashHighlighter.flash(this);
}

/*------------------------------------------------------------------+
| The callback function to enumerate all window properties.         |
| The Window object that called EnumPropsEx must be passed as the   |
| third parameter (lParam).                                         |
+------------------------------------------------------------------*/
BOOL CALLBACK Window::enumProps(HWND hwnd, LPWSTR string,
                                HANDLE hData, ULONG_PTR userData) {
    WindowPropList* list = reinterpret_cast<WindowPropList*>(userData);

    // Name can be either a string or an ATOM (int)
    String name = IS_INTRESOURCE(string) ? 
                     hexString((uint)string) + " (Atom)" :
                     String::fromWCharArray(string);
    list->append(WindowProp(name, hData));

    // Return TRUE to continue enumeration, FALSE to stop.
    return TRUE;
}