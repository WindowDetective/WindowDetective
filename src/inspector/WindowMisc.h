/////////////////////////////////////////////////////////////////////
// File: WindowMisc.h                                              //
// Date: 14/2/10                                                   //
// Desc: Miscellaneous window related class definitions            //
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

#ifndef WINDOW_MISC_H
#define WINDOW_MISC_H

#include "window_detective/Error.h"
#include "inspector/RemoteFunctions.h"

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
    ~WindowClassStyle() {}

    String getName() { return name; }
    ulong getValue() { return (ulong)value; }
};


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

    static String nameForId(UINT id);
    String getName() const;
    LRESULT send();
};


/*------------------------------------------------------------------+
| Represents a type of window or control, e.g. Button, Static, etc  |
+------------------------------------------------------------------*/
class WindowClass {
protected:
    String name;                // Name of the window class
    String displayName;         // The "user-friendly" name (for system classes)
    bool native;                // Native system control or subclassed
    // TODO: Use pointer to my Process class instead
    HINSTANCE creatorInst;      // Application that created the class
    QIcon icon;                 // An icon which represents this window type
    WindowClassStyleList styles;// List of styles applied to this window
    uint classExtraBytes;       // Extra memory allocated to class
    uint windowExtraBytes;      // Extra memory allocated to each window instance
    WinBrush* backgroundBrush;  // For painting window's background
    WindowStyleList applicableWindowStyles;

public:
    WindowClass() {}
    WindowClass(String name);
    WindowClass(String name, String displayName, bool isNative = true);
    ~WindowClass();

    String getName() { return name; }
    String getDisplayName();
    bool isNative() { return native; }
    HINSTANCE getCreatorInst() { return creatorInst; }
    const QIcon& getIcon() { return icon; }
    WindowClassStyleList getStyles() { return styles; }
    uint getClassExtraBytes() { return classExtraBytes; }
    uint getWindowExtraBytes() { return windowExtraBytes; }
    WinBrush* getBackgroundBrush() { return backgroundBrush; }
    WindowStyleList getApplicableWindowStyles() { return applicableWindowStyles; }
    void addApplicableStyle(WindowStyle* s) { applicableWindowStyles.append(s); }

    void updateInfoFrom(WindowInfoStruct* info);
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

    WinBrush(HBRUSH handle, LOGBRUSH brush) :
        handle(handle) {
        style = brush.lbStyle;
        colour = brush.lbColor;
        hatchType = brush.lbHatch;
    }

    String getStyleName() {
        return Resources::getConstantName("BrushStyles", style);
    }

    String getHatchName() {
        return Resources::getConstantName("HatchStyles", hatchType);
    }
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

    WinFont(HFONT handle, LOGFONTW font) :
        handle(handle) {
        faceName = String::fromWCharArray(font.lfFaceName, -1);
        width = font.lfWidth;
        height = font.lfHeight;
        weight = font.lfWeight;
        quality = font.lfQuality;
        style = (font.lfItalic & 0x01) |
               ((font.lfUnderline & 0x01) << 1) |
               ((font.lfStrikeOut & 0x01) << 2);
    }

    String getWeightName() {
        if (Resources::hasConstant("FontWeights", weight)) {
            return Resources::getConstantName("FontWeights", weight);
        }
        else {
            return "";
        }
    }

    String getQualityName() {
        return Resources::getConstantName("FontQuality", quality);
    }

    String getStyleString() {
        String s;

        if (!style) return "normal";
        if (style & 0x01) {
            if (!s.isEmpty()) s += ", ";
            s += "italic";
        }
        if (style & 0x02) {
            if (!s.isEmpty()) s += ", ";
            s += "underline";
        }
        if (style & 0x04) {
            if (!s.isEmpty()) s += ", ";
            s += "strike-out";
        }
        return s;
    }
};


class TimeoutError : public Error {
public:
    TimeoutError() : Error("Timeout Error") {}
    TimeoutError(const WindowMessage& message);
};

};   // namespace inspector

#endif   // WINDOW_MISC_H