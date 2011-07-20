/////////////////////////////////////////////////////////////////////
// File: WindowMisc.h                                              //
// Date: 14/2/10                                                   //
// Desc: Miscellaneous window related class definitions            //
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

#ifndef WINDOW_MISC_H
#define WINDOW_MISC_H

#include "window_detective/Error.h"
#include "inspector/RemoteFunctions.h"
#include "hook/Hook.h"

namespace inspector {

/*------------------------------------------------------------------+
| Represents a specific window style flag, used to tell a control   |
| what type it is, how it should behave, and any other style        |
| information.                                                      |
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
    WindowStyle(bool isGeneric = true);
    WindowStyle(const WindowStyle& other);
    ~WindowStyle() {}

    void readFrom(QStringList values);
    String getName() { return name; }
    ulong getValue() { return (ulong)value; }
    String getDescription() { return description; }
    bool isExtended() { return extended; }
    bool isValidFor(WindowClass* windowClass);
};


/*------------------------------------------------------------------+
| Represents a specific class style flag, used to define            |
| additional elements of the window class.                          |
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
    WindowClassStyle(const WindowClassStyle& other);
    ~WindowClassStyle() {}

    String getName() const { return name; }
    ulong getValue() const { return (ulong)value; }
};


/*------------------------------------------------------------------+
| The definition of a window message.                               |
+------------------------------------------------------------------*/
//// Probably won't need this
/*class WindowMessageDefn {
private:
    uint id;
    String name;
    WindowClass* applicableClass;  // NULL means it's applicable to all

public:
    WindowMessageDefn() {}
    WindowMessageDefn(UINT id, String name) :
        id(id), name(name), applicableClass(NULL) {}
    ~WindowMessage() {}

    uint getId() const { return id; }
    String getName() const { return name; }
    void toXmlStream(QXmlStreamWriter& stream);
};*/


/*------------------------------------------------------------------+
| Represents an event message that is sent to a window              |
+------------------------------------------------------------------*/
class WindowMessage {
public:
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

    static String nameForId(UINT id, WindowClass* windowClass = NULL);
    String getName(WindowClass* windowClass = NULL) const;
    LRESULT send();
    void toXmlStream(QXmlStreamWriter& stream);
};


/*------------------------------------------------------------------+
| Represents a type of window or control, e.g. Button, Static, etc  |
+------------------------------------------------------------------*/
class WindowClass {
protected:
    String name;                 // Name of the window class
    String friendlyName;         // The "user-friendly" name (for system classes)
    bool native;                 // Native system control or subclassed
    QIcon icon;                  // An icon which represents this window type
    WindowClassStyleList styles; // List of styles applied to this window
    uint classExtraBytes;        // Extra memory allocated to class
    uint windowExtraBytes;       // Extra memory allocated to each window instance
    WinBrush* backgroundBrush;   // For painting window's background
    WindowStyleList applicableStyles;
    QHash<uint,String> windowMessageNames;

public:
    WindowClass() {}
    WindowClass(String name);
    WindowClass(String name, String friendlyName, bool isNative = true);
    WindowClass(const WindowClass& other);
    ~WindowClass();

    String getName() const { return name; }
    String getDisplayName();
    bool isNative() const { return native; }
    const QIcon getIcon() const { return icon; }
    WindowClassStyleList getStyles() const { return styles; }
    uint getClassExtraBytes() const { return classExtraBytes; }
    uint getWindowExtraBytes() const { return windowExtraBytes; }
    WinBrush* getBackgroundBrush() const { return backgroundBrush; }
    WindowStyleList getApplicableStyles() const { return applicableStyles; }
    QHash<uint,String> getApplicableMessages() const { return windowMessageNames; }
    void addApplicableStyle(WindowStyle* s);
    void addApplicableMessage(uint id, String name);

    void updateInfoFrom(WindowInfoStruct* info);
    void toXmlStream(QXmlStreamWriter& stream) const;
};


/*------------------------------------------------------------------+
| Represents a window property as used by the Get/SetProp API.      |
+------------------------------------------------------------------*/
class WindowProp {
public:
    String name;
    HANDLE data;

    WindowProp(String name, HANDLE data) :
        name(name), data(data) {}

    void toXmlStream(QXmlStreamWriter& stream) const;
};


/*------------------------------------------------------------------+
| A Window's GDI brush (HBRUSH/LOGBRUSH)                            |
+------------------------------------------------------------------*/
class WinBrush {
public:
    HBRUSH handle;
    UINT style;
    COLORREF colour;
    int hatchType;

    WinBrush(HBRUSH handle, LOGBRUSH brush);

    String getStyleName() const;
    String getHatchName() const;
    void toXmlStream(QXmlStreamWriter& stream) const;
};


/*------------------------------------------------------------------+
| A Window's GDI font (HFONT/LOGFONT)                               |
+------------------------------------------------------------------*/
class WinFont {
public:
    HFONT handle;
    String faceName;
    int width, height;
    byte weight;
    byte style;     // bitfield containing italic, underline, strikeOut
    byte quality;

    WinFont(HFONT handle, LOGFONTW font);
    WinFont(const WinFont& other);

    String getWeightName() const;
    String getQualityName() const;
    String getStyleString() const;
    void toXmlStream(QXmlStreamWriter& stream) const;
};

};   // namespace inspector

#endif   // WINDOW_MISC_H