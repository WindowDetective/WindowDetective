/////////////////////////////////////////////////////////////////////
// File: Window.cpp                                                //
// Date: 20/2/10                                                   //
// Desc: Creates mock window objects from a Windows handle (HWND)  //
//   as well as other window related objects such as window        //
//   classes, styles and messages.                                 //
/////////////////////////////////////////////////////////////////////

#include "inspector.h"
#include "WindowManager.h"
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
using namespace inspector;

/*------------------------------------------------------------------+
 | Constructor                                                      |
 | Creates a mock Window object from the real window handle         |
 +------------------------------------------------------------------*/
Window::Window(HWND handle) :
    handle(handle) {
}

/*------------------------------------------------------------------+
 | Send a message to the window with the given parameters           |
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
 | Send a message to the window with no parameters. Mostly used     |
 | for getting a value from the window or for sending an action     |
 | message (e.g. WM_CLOSE).                                         |
 +------------------------------------------------------------------*/
template <class ReturnType>
ReturnType Window::sendMessage(UINT msg) {
    return sendMessage<ReturnType,int,int>(msg, NULL, NULL);
}

/*------------------------------------------------------------------+
 | If this is a child window, then it's coords are relative to it's |
 | parent. If not then they are the same as it's absolute coords.   |
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
 | Returns all child windows who's ancestor is this.                |
 +------------------------------------------------------------------*/
QList<Window*> Window::getDescendants() {
    QList<Window*> allChildren;
    foreach (Window* child, children) {
        allChildren.append(child);
        allChildren.append(child->getDescendants());
    }
    return allChildren;
}

/*------------------------------------------------------------------+
 | Returns a string suitable for display in the UI.                 |
 +------------------------------------------------------------------*/
String Window::displayName() {
    if (!windowClass)
        return hexString((uint)handle);
    else
        return windowClass->getName()+" ("+hexString((uint)handle)+")";
}


/**********************/
/*** Update methods ***/
/**********************/

void Window::fireUpdateEvent(UpdateReason reason) {
    emit updated(reason);
}

/*------------------------------------------------------------------+
 | Updates all properties such as title, size, position and window  |
 | class by requesting it from the real window.                     |
 +------------------------------------------------------------------*/
void Window::update() {
    updateText();
    updateWindowClass();
    updateWindowInfo();
    updateFlags();
    updateIcon();
}

/*------------------------------------------------------------------+
 | Updates the window title or control text by sending the          |
 | WM_GETTEXT message to the window.                                |
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
            text = String::fromWCharArray(charData);
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
 | Updates the window's class and adds it to the WindowManager's    |
 | list if it does not already exist.                               |
 +------------------------------------------------------------------*/
void Window::updateWindowClass() {
    WCHAR* charData = new WCHAR[255];

    // If it fails, try with a bigger buffer
    if (!GetClassName(handle, charData, 255)) {
        delete[] charData;
        charData = new WCHAR[1024];
        GetClassName(handle, charData, 1024);
    }
    String className = String::fromWCharArray(charData);
    delete[] charData;

    // Find existing class or add it as a new one
    WindowManager* manager = WindowManager::current();
    if (manager->allWindowClasses.contains(className)) {
        windowClass = manager->allWindowClasses[className];
    }
    else {
        windowClass = new WindowClass(className);
        manager->allWindowClasses.insert(className, windowClass);
    }
}

/*------------------------------------------------------------------+
 | Updates various window info including window and client area     |
 | rectangle, styles, status and atom type.                         |
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
 | Updates various flags that indicate the window's status such as  |
 | whether it is visible or enabled.                                |
 +------------------------------------------------------------------*/
void Window::updateFlags() {
    visible = IsWindowVisible(handle);
    enabled = IsWindowEnabled(handle);
    unicode = IsWindowUnicode(handle);
}

/*------------------------------------------------------------------+
 | Updates the window's small and large icon. If no small or large  |
 | icon has been set, the icon that represents this class is used.  |                                                 |
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
                Logger::osWarning(TR("Failed to get small icon for window ")+displayName());
            }
        }
        if (largeIcon) {
            bool result = GetIconInfo(largeIcon, &iconInfo);
            if (result) {
                icon.addPixmap(QPixmap::fromWinHICON(largeIcon));
            }
            else {
                Logger::osWarning(TR("Failed to get large icon for window ")+displayName());
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


/**********************/
/*** Setter methods ***/
/**********************/

void Window::setText(String newText) {
    const ushort* charData = newText.utf16();
    LRESULT result = sendMessage<LRESULT,int,const ushort*>(WM_SETTEXT, NULL, charData);
    if (!result && !GetLastError()) {
        Logger::osError(TR("Unable to set window text: ")+newText);
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

void Window::setEnabled(bool isEnabled) {
    EnableWindow(handle, isEnabled);
}


/***********************/
/*** Command methods ***/
/***********************/

/*------------------------------------------------------------------+
 | Shows the window in front of all others. If 'activate' is true,  |
 | the window will become the active one. If 'stay' is true, the    |
 | window will stay on top always.                                  |
 +------------------------------------------------------------------*/
void Window::show(bool activate, bool stay) {
    HWND insertPos = stay ? HWND_TOPMOST : HWND_TOP;
    if (activate) {
        BOOL result = SetWindowPos(handle, insertPos, 0, 0, 0, 0,
                        SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
        if (!result) {
            Logger::osError(TR("Unable to show window ")+displayName());
        }
    }
    else {
        ShowWindow(handle, SW_SHOWNA); // Show not active (possibly maximized)
    }
}

void Window::hide() {
    ShowWindow(handle, SW_HIDE);
}

void Window::maximise() {
    ShowWindow(handle, SW_SHOWMAXIMIZED);
}

void Window::minimise() {
    ShowWindow(handle, SW_FORCEMINIMIZE);
}

void Window::close() {
    try {
        LRESULT result = sendMessage<LRESULT,int,int>(WM_CLOSE, NULL, NULL);
        if (result != 0) {
            Logger::info(TR("Window (") + displayName() +
                         TR(") returned from WM_CLOSE with value ") +
                         String::number(result));
        }
    }
    catch (TimeoutError e) {
        Logger::error(e);
    }
}

void Window::destroy() {
    // TODO: Use DestroyWindow. Must be called from remote thread
}
