/////////////////////////////////////////////////////////////////////
// File: StringRenderer.cpp                                        //
// Date: 25/3/10                                                   //
// Desc: Provides functions for converting various data types and  //
//   objects to a string representation for display.               //
/////////////////////////////////////////////////////////////////////

#include "StringRenderer.h"


/*** These functions return a plain string representation ***/

String stringLabel(int value) {
    return String::number(value);
}

String stringLabel(bool value) {
    return value ? "true" : "false";
}

String stringLabel(String str) {
    return escapeHtml(str);
}

String stringLabel(HWND hwnd) {
    return hexString((uint)hwnd);
}

String stringLabel(const QRect& rect) {
    return "(" + String::number(rect.left()) + ", " +
                 String::number(rect.top()) + ", " +
                 String::number(rect.right()) + ", " +
                 String::number(rect.bottom()) + ")";
}

String stringLabel(const QPoint& pos) {
    return "(" + String::number(pos.x()) + ", " +
                 String::number(pos.y()) + ")";
}

String stringLabel(const QSize& size) {
    return "(" + String::number(size.width()) + ", " +
                 String::number(size.height()) + ")";
}

String stringLabel(WindowClass* windowClass) {
    return windowClass ? windowClass->getDisplayName() : "";
}

String stringLabel(const WindowStyleList& list) {
    String value;
    QTextStream stream(&value);
    WindowStyleList::const_iterator i;

    for (i = list.begin(); i != list.end(); ++i) {
        stream << (*i)->getName() << '\n';
    }
    return value;
}


/*** These functions return a string in HTML format ***/

String htmlLabel(const WindowStyleList& list) {
    String value;
    QTextStream stream(&value);
    WindowStyleList::const_iterator i;

    for (i = list.begin(); i != list.end(); ++i) {
        stream << "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px;"
                  "margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
               << (*i)->getName() << "</p>";
    }
    return value;
}