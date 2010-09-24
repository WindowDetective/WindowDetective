/////////////////////////////////////////////////////////////////////
// File: StringFormatter.h                                         //
// Date: 25/3/10                                                   //
// Desc: Provides functions for converting various data types and  //
//   objects to a string representation for display.               //
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

#ifndef STRING_FORMATTER_H
#define STRING_FORMATTER_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
using namespace inspector;

template <class T>
class StringFormatter {
private:
    T value;
public:
    StringFormatter(T value) : value(value) {}
    ~StringFormatter() {}

    String stringLabel() {
        return QVariant::fromValue(value).toString();
    }
    String htmlLabel() { return stringLabel(); }
};

template <>
class StringFormatter<bool> {
private:
    bool value;
public:
    StringFormatter<bool>(bool b) : value(b) {}

    String stringLabel() { return value ? "true" : "false"; }
    String htmlLabel() { return stringLabel(); }
};

template <>
class StringFormatter<String> {
private:
    String value;
public:
    StringFormatter<String>(String s) : value(s) {}

    String stringLabel() { return value; }
    String htmlLabel() { return Qt::escape(value); }
};

template <>
class StringFormatter<HWND> {
private:
    HWND value;
public:
    StringFormatter<HWND>(HWND hwnd) : value(hwnd) {}

    String stringLabel() { return hexString((uint)value); }
    String htmlLabel() { return stringLabel(); }
};

template <>
class StringFormatter<QRect> {
private:
    QRect value;
public:
    StringFormatter<QRect>(QRect rect) : value(rect) {}

    String stringLabel() {
        return "(" + String::number(value.left()) + ", " +
                     String::number(value.top()) + ", " +
                     String::number(value.right()) + ", " +
                     String::number(value.bottom()) + ")";
    }
    String htmlLabel() { return stringLabel(); }
};

template <>
class StringFormatter<QPoint> {
private:
    QPoint value;
public:
    StringFormatter<QPoint>(QPoint point) : value(point) {}

    String stringLabel() {
        return "(" + String::number(value.x()) + ", " +
                     String::number(value.y()) + ")";
    }
    String htmlLabel() { return stringLabel(); }
};

template <>
class StringFormatter<QSize> {
private:
    QSize value;
public:
    StringFormatter<QSize>(QSize size) : value(size) {}

    String stringLabel() {
        return "(" + String::number(value.width()) + ", " +
                     String::number(value.height()) + ")";
    }
    String htmlLabel() { return stringLabel(); }
};

template <>
class StringFormatter<QColor> {
private:
    QColor value;
public:
    StringFormatter<QColor>(QColor colour) : value(colour) {}

    String stringLabel() {
        // TODO: Use setting to choose between r,g,b and hex
        String str;
        QTextStream stream(&str);
        stream << "(" << String::number(value.red()) << ", "
               << String::number(value.green()) << ", "
               << String::number(value.blue()) << ")";
        return str;
    }

    String htmlLabel() { 
        String resultStr, colourStr;
        QTextStream stream1(&colourStr);
        stream1 << "(" << String::number(value.red()) << ", "
                << String::number(value.green()) << ", "
                << String::number(value.blue()) << ")";

        // Html div element will have this colour as it's background
        // colour. If the average of all channels is less than 140,
        // then the text colour will be white, else it is black
        int avg = (value.red() + value.green() + value.blue()) / 3;
        QTextStream stream2(&resultStr);
        stream2 << "<div style=\"background-color: rgb"
                << colourStr << "; color: rgb("
                << (avg < 140 ? "255, 255, 255" : "0, 0, 0")
                << ");\">" << colourStr << "</div>";
        return resultStr;
    }
};

template <>
class StringFormatter<COLORREF> {
private:
    COLORREF value;
public:
    StringFormatter<COLORREF>(COLORREF colour) : value(colour) {}

    String stringLabel() {
        return StringFormatter<QColor>(QColorFromCOLORREF(value)).stringLabel();
    }
    String htmlLabel() {
        return StringFormatter<QColor>(QColorFromCOLORREF(value)).htmlLabel();
    }
};

template <>
class StringFormatter<WindowClass*> {
private:
    WindowClass* value;
public:
    StringFormatter<WindowClass*>(WindowClass* wndClass) : value(wndClass) {}

    String stringLabel() { return value ? value->getDisplayName() : ""; }
    String htmlLabel() { return Qt::escape(stringLabel()); }
};

template <>
class StringFormatter<WindowStyleList> {
private:
    WindowStyleList value;
public:
    StringFormatter<WindowStyleList>(WindowStyleList list) : value(list) {}

    String stringLabel() {
        String str;
        QTextStream stream(&str);
        WindowStyleList::const_iterator i;

        for (i = value.begin(); i != value.end(); ++i) {
            stream << (*i)->getName() << '\n';
        }
        return str;
    }

    String htmlLabel() {
        String str;
        QTextStream stream(&str);
        WindowStyleList::const_iterator i;

        for (i = value.begin(); i != value.end(); ++i) {
            stream << "<p style=\" margin-top:0px; margin-bottom:0px;"
                      "margin-left:0px; margin-right:0px;"
                      "-qt-block-indent:0; text-indent:0px;\">"
                   << (*i)->getName() << "</p>";
        }
        return str;
    }
};

template <>
class StringFormatter<WindowPropList> {
private:
    WindowPropList value;
public:
    StringFormatter<WindowPropList>(WindowPropList list) : value(list) {}

    String stringLabel() {
        String str;
        QTextStream stream(&str);
        WindowPropList::const_iterator i;

        for (i = value.begin(); i != value.end(); ++i) {
            stream << (*i).name << '\t'
                   << hexString((int)(*i).data) << '\n';
        }
        return str;
    }

    String htmlLabel() {
        String str;
        QTextStream stream(&str);
        WindowPropList::const_iterator i;

        stream << "<table><tr>"
               << "<th class=\"sub\">Name</th><th class=\"sub\">Data</th>"
               << "</tr>";
        for (i = value.begin(); i != value.end(); ++i) {
            stream << "<tr><td>" << (*i).name
                   << "</td><td>" << hexString((int)(*i).data)
                   << "</td></tr>";
        }
        stream << "</table>";
        return str;
    }
};

template <>
class StringFormatter<WinBrush*> {
private:
    WinBrush* value;
public:
    StringFormatter<WinBrush*>(WinBrush* brush) : value(brush) {}

    String stringLabel() {
        if (!value || !value->handle) return QObject::tr("none");

        // Check if the handle is actually a colour id
        uint id = (uint)(value->handle) - 1;
        if (Resources::hasConstant("SystemColours", id)) {
            return Resources::getConstantName("SystemColours", id);
        }

        String str;
        QTextStream stream(&str);
        StringFormatter<COLORREF> colourFormatter(value->colour);
        stream << "Handle:\t" << hexString((uint)value->handle)
               << "\nStyle:\t" << value->getStyleName()
               << "\nColour:\t" << colourFormatter.stringLabel()
               << "\nHatch:\t" << value->getHatchName();
        return str;
    }

    String htmlLabel() { 
        if (!value || !value->handle) return QObject::tr("none");

        // Check if the handle is actually a colour id
        uint id = (uint)(value->handle) - 1;
        if (Resources::hasConstant("SystemColours", id)) {
            return Resources::getConstantName("SystemColours", id);
        }

        String str;
        QTextStream stream(&str);
        StringFormatter<COLORREF> colourFormatter(value->colour);
        stream << "<table>"
               << "<tr><td>Handle:</td><td>" << hexString((uint)value->handle) << "</td></tr>"
               << "<tr><td>Style:</td><td>" << value->getStyleName() << "</td></tr>"
               << "<tr><td>Colour:</td><td>" << colourFormatter.htmlLabel() << "</td></tr>"
               << "<tr><td>Hatch:</td><td>" << value->getHatchName() << "</td></tr>"
               << "</table>";
        return str;
    }
};

template <>
class StringFormatter<WinFont*> {
private:
    WinFont* value;
public:
    StringFormatter<WinFont*>(WinFont* font) : value(font) {}

    String stringLabel() {
        if (!value) return QObject::tr("none");
        if (!value->handle) return QObject::tr("system font");

        String str, weightString, sizeString;
        weightString = value->getWeightName();
        if (weightString.isEmpty()) {
            weightString = String::number(value->weight);
        }
        else {
            weightString = weightString+" ("+String::number(value->weight)+")";
        }
        sizeString = String::number(value->width)+", "+String::number(value->height);
        QTextStream stream(&str);
        stream << "Handle:\t" << hexString((uint)value->handle)
               << "\nFace name:\t" << value->faceName
               << "\nWeight:\t" << weightString
               << "\nWidth/Height:\t" << sizeString
               << "\nQuality:\t" << value->getQualityName()
               << "\nStyle:\t" << value->getStyleString();
        return str;
    }

    String htmlLabel() { 
        if (!value) return QObject::tr("none");
        if (!value->handle) return QObject::tr("system font");

        String str, weightString, sizeString;
        weightString = value->getWeightName();
        if (weightString.isEmpty()) {
            weightString = String::number(value->weight);
        }
        else {
            weightString = weightString+" ("+String::number(value->weight)+")";
        }
        sizeString = String::number(value->width)+", "+String::number(value->height);
        QTextStream stream(&str);
        stream << "<table>"
               << "<tr><td>Handle:</td><td>" << hexString((uint)value->handle) << "</td></tr>"
               << "<tr><td>Face name:</td><td>" << value->faceName << "</td></tr>"
               << "<tr><td>Weight:</td><td>" << weightString << "</td></tr>"
               << "<tr><td>Width/Height:</td><td>" << sizeString << "</td></tr>"
               << "<tr><td>Quality:</td><td>" << value->getQualityName() << "</td></tr>"
               << "<tr><td>Style:</td><td>" << value->getStyleString() << "</td></tr>"
               << "</table>";
        return str;
    }
};


// TODO: Remove these and replace them with StringFormatter
/*** These functions return a plain string representation ***/

// Primitive and built-in types
// TODO: Is there any way to use a single function for all primitives?
String stringLabel(int value);
inline String stringLabel(uint value) { return stringLabel((int)value); }
String stringLabel(bool value);
inline String stringLabel(String value) { return value; }

// Objects and typedefs
String stringLabel(HWND value);
String stringLabel(const QRect& value);
String stringLabel(const QPoint& value);
String stringLabel(const QSize& value);
String stringLabel(WindowClass* value);
String stringLabel(const WindowStyleList& value);


/*** These functions return a string in HTML format ***/

inline String htmlLabel(int value) { return stringLabel(value); }
inline String htmlLabel(uint value) { return stringLabel(value); }
inline String htmlLabel(bool value) { return stringLabel(value); }
String htmlLabel(String value);

inline String htmlLabel(HWND value) { return stringLabel(value); }
inline String htmlLabel(const QRect& value) { return stringLabel(value); }
inline String htmlLabel(const QPoint& value) { return stringLabel(value); }
inline String htmlLabel(const QSize& value) { return stringLabel(value); }
inline String htmlLabel(WindowClass* value) { return stringLabel(value); }
String htmlLabel(const WindowStyleList& value);
String htmlLabel(const WindowPropList& value);

#endif   // STRING_FORMATTER_H