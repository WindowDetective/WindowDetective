/////////////////////////////////////////////////////////////////////
// File: window_misc.h                                             //
// Date: 14/2/10                                                   //
// Desc: Miscellaneous window related class definitions            //
/////////////////////////////////////////////////////////////////////

#ifndef WINDOW_MISC_H
#define WINDOW_MISC_H

#include "window_detective/Error.h"

namespace inspector {

typedef QList<WindowStyle*> WindowStyleList;
typedef QList<WindowClassStyle*> WindowClassStyleList;


/*------------------------------------------------------------------+
 | Represents a specific window style flag, used to tell a control  |
 | what type it is, how it should behave, and any other style       |
 | information.                                                     |
 +------------------------------------------------------------------*/
class WindowStyle {
private:
    String name;             // Name of the style flag as used in the Windows API, e.g. WS_VISIBLE
    DWORD value;             // The value of the flag (should be a power of two)
    bool extended;           // If the style is an extended (WS_EX_*) style
    bool isGeneric;          // True if it applies to any window class
    DWORD depends;           // Only valid if window also includes these styles
    DWORD excludes;          // Only valid if these aren't set on window
    String description;      // Helpful info on this style

public:
    WindowStyle(bool isGeneric = true, bool isExtended = false);
    ~WindowStyle() {}

    void readFrom(QStringList values);
    String getName() { return name; }
    ulong getValue() { return (ulong)value; }
    String getDescription() { return description; }
    bool isExtended() { return extended; }
    bool isValidFor(WindowClass* windowClass);
};


/*------------------------------------------------------------------+
 | Represents a specific class style flag, used to define           |
 | additional elements of the window class.                         |
 +------------------------------------------------------------------*/
class WindowClassStyle {
private:
    String name;             // Name of the style flag as used in the Windows API
    DWORD value;             // The value of the flag
    String description;      // Helpful info on this style

public:
    WindowClassStyle() : value(0) {}
    WindowClassStyle(String name,
                     uint value,
                     String description);
    ~WindowClassStyle() {}

    String getName() { return name; }
    ulong getValue() { return (ulong)value; }
};


/*------------------------------------------------------------------+
 | Represents an event message that is sent to a window             |
 +------------------------------------------------------------------*/
class WindowMessage {
public:
    String name;             // Name of the message as used in the Windows API
    Window* window;          // Window to recieve message
    UINT id;                 // Message id
    WPARAM wParam;           // 1st parameter
    LPARAM lParam;           // 2nd parameter
    LRESULT returnValue;     // Return value from whoever handled this

    WindowMessage() {}
    WindowMessage(HWND hWnd, UINT id,
                  WPARAM wParam, LPARAM lParam,
                  LRESULT returnValue = 0);
    WindowMessage(Window* window, UINT id,
                  WPARAM wParam, LPARAM lParam,
                  LRESULT returnValue = 0);
    ~WindowMessage() {}
    void initName();
    LRESULT send();
};


/*------------------------------------------------------------------+
 | Represents a type of window or control, e.g. Button, Static, etc |
 +------------------------------------------------------------------*/
class WindowClass {
protected:
    String name;             // Name of the window class
    String displayName;      // The "user-friendly" name (for built-in Win32 classes)
    bool native;             // Native Win32 control or subclassed
    // TODO: Use pointer to my Process class instead
    HINSTANCE creatorInst;   // Application that created the class
    QIcon icon;              // An icon which represents this window type
    WindowClassStyleList styles;// List of styles applied to this window
    uint classExtraBytes;    // Extra memory allocated to class
    uint windowExtraBytes;   // Extra memory allocated to window instance
    WindowStyleList applicableWindowStyles;

public:
    WindowClass() {}
    WindowClass(String name);
    WindowClass(String name, String displayName, bool isNative = true);
    ~WindowClass() {}

    String getName() { return name; }
    String getDisplayName();
    bool isNative() { return native; }
    HINSTANCE getCreatorInst() { return creatorInst; }
    const QIcon& getIcon() { return icon; }
    WindowClassStyleList getStyles() { return styles; }
    uint getClassExtraBytes() { return classExtraBytes; }
    uint getWindowExtraBytes() { return windowExtraBytes; }
    WindowStyleList getApplicableWindowStyles() { return applicableWindowStyles; }
    void addApplicableStyle(WindowStyle* s) { applicableWindowStyles.append(s); }
};

class TimeoutError : public Error {
public:
    TimeoutError() : Error("Timeout Error") {}
    TimeoutError(const WindowMessage& message);
};

};   // namespace inspector

#endif   // WINDOW_MISC_H