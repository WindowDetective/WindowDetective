/////////////////////////////////////////////////////////////////////
// File: include.h                                                 //
// Date: 14/2/10                                                   //
// Desc: This file includes all header files from each component.  //
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

#ifndef _INCLUDE_H
#define _INCLUDE_H

#include <Windows.h>
#include <Commctrl.h>
#include <Psapi.h>
#include <QtGui\QtGui>
#include <QtXml\QtXml>
#include "resource.h"

typedef unsigned char byte;
typedef QString String;

// Random number between 0.0 and 1.0
#define RAND                   (float(rand())/float(RAND_MAX))
#define SIN(min, max, angle)   (((min+max)/2) + (((max-min)/2)*sin(angle)))

// Random range between 0 and x or a and b.
static inline float rand(float x) { return RAND*x; }
static inline float rand(float a, float b) {return a + RAND*(b-a); }
static inline int rand(int x) { return (int)(RAND*x); }
static inline int rand(int a, int b) {return (int)(a + RAND*(b-a)); }

// Size of static array
#define arraysize(a) (sizeof(a)/sizeof(a[0]))

/* Bitfield functions */
#define SET_BIT(var, num)    ((var) |=  (1 << (num)))
#define CLR_BIT(var, num)    ((var) &= ~(1 << (num)))
#define XOR_BIT(var, num)    ((var) ^=  (1 << (num)))
#define GET_BIT(var, num)    ((var) &   (1 << (num)))
#define TEST_BITS(var, mask) (((var) & (mask)) == (mask))

// For use in non-Qt classes
#define TR(text) (QObject::tr(text))


inline bool isShiftDown() { return GetKeyState(VK_SHIFT) < 0; }
inline bool isCtrlDown() { return GetKeyState(VK_CONTROL) < 0; }

/*------------------------------------------------------------------+
| Returns an integer indicating the version of the operating        |
| system this application is running on. XP is 501.                 |
+------------------------------------------------------------------*/
inline int getOSVersion() {
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&info);
    return (info.dwMajorVersion * 100) + info.dwMinorVersion;
}

/*------------------------------------------------------------------+
| Returns a string of num in hexadecimal format, padded to 8 digits |
+------------------------------------------------------------------*/
inline String hexString(int num) {
    String hex = String::number((uint)num, 16).toUpper();
    if (hex.size() < 8) {
        hex = String(8 - hex.size(), '0') + hex;
    }
    return "0x" + hex;
}

/*------------------------------------------------------------------+
| Parse the given string to a colour in the form                    |
| "red,green,blue[,alpha]". If alpha is omitted, it's set to 255.   |
| Returns the default colour red on error.                          |
+------------------------------------------------------------------*/
inline QColor stringToColour(String string) {
    QColor colour;
    QStringList rgbList = string.split(",");
    if (rgbList.size() != 3 && rgbList.size() != 4)
        goto error;     // goto! :O
    bool isOk;
    colour.setRed(rgbList[0].toInt(&isOk));
    if (!isOk) goto error;
    colour.setGreen(rgbList[1].toInt(&isOk));
    if (!isOk) goto error;
    colour.setBlue(rgbList[2].toInt(&isOk));
    if (!isOk) goto error;
    if (rgbList.size() == 4) {
        colour.setAlpha(rgbList[3].toInt(&isOk));
        if (!isOk) goto error;
    }
    else {
        colour.setAlpha(255);
    }
    return colour;          // If everything went ok

    error:
    return QColor();  // Return invalid colour on error
}

/*------------------------------------------------------------------+
| Return a string representation of the colour in the form          |
| "red,green,blue[,alpha]". If alpha is 255, it is omitted.         |
+------------------------------------------------------------------*/
inline String colourToString(QColor colour) {
    String s = String::number(colour.red()) + "," +
               String::number(colour.green()) + "," +
               String::number(colour.blue());
    if (colour.alpha() != 255)
        s += "," + String::number(colour.alpha());
    return s;
}


/*------------------------------------------------------------------+
| Helper functions for converting between basic Windows and Qt types|
+------------------------------------------------------------------*/

inline QPoint QPointFromPOINT(const POINT& p) {
    return QPoint(static_cast<int>(p.x), static_cast<int>(p.y));
}

inline POINT POINTFromQPoint(const QPoint& p) {
    POINT winPoint;
    winPoint.x = static_cast<LONG>(p.x());
    winPoint.y = static_cast<LONG>(p.y());
    return winPoint;
}

// MSDN: By convention, the right and bottom edges of the rectangle are
// normally considered exclusive. In other words, the pixel whose coordinates
// are ( right, bottom ) lies immediately outside of the the rectangle.
inline QRect QRectFromRECT(const RECT& rect) {
    return QRect(QPoint(static_cast<int>(rect.left),
                        static_cast<int>(rect.top)),
                 QPoint(static_cast<int>(rect.right-1),
                        static_cast<int>(rect.bottom-1)));
}

inline RECT RECTFromQRect(const QRect& rect) {
    RECT winRect;
    winRect.left = static_cast<LONG>(rect.left());
    winRect.top = static_cast<LONG>(rect.top());
    winRect.right = static_cast<LONG>(rect.right()+1);
    winRect.bottom = static_cast<LONG>(rect.bottom()+1);
    return winRect;
}

inline QColor QColorFromCOLORREF(COLORREF rgb) {
    return QColor(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
}

inline COLORREF COLORREFFromQColor(const QColor& colour) {
    return RGB(colour.red(), colour.green(), colour.blue());
}

#endif  // BALLISTIC_INCLUDE_H
