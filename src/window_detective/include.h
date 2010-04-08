/////////////////////////////////////////////////////////////////////
// File: include.h                                                 //
// Date: 14/2/10                                                   //
// Desc: This file includes all header files from each component.  //
/////////////////////////////////////////////////////////////////////

#ifndef _INCLUDE_H
#define _INCLUDE_H

#include <Windows.h>
#include <Psapi.h>
#include <QtGui\QtGui>
#include "resource.h"

typedef unsigned char uchar;
typedef unsigned char byte;
typedef unsigned char ubyte;
typedef unsigned int  uint;
typedef QString String;

// Random number between 0.0 and 1.0
#define RAND                   (float(rand())/float(RAND_MAX))
#define SIN(min, max, angle)   (((min+max)/2) + (((max-min)/2)*sin(angle)))

// Random range between 0 and x or a and b.
static inline float rand(float x) { return RAND*x; }
static inline float rand(float a, float b) {return a + RAND*(b-a); }
static inline int rand(int x) { return (int)(RAND*x); }
static inline int rand(int a, int b) {return (int)(a + RAND*(b-a)); }

/* Bitfield functions */
#define SET_BIT(var, num)  (var |=  (1 << num))
#define CLR_BIT(var, num)  (var &= ~(1 << num))
#define XOR_BIT(var, num)  (var ^=  (1 << num))
#define GET_BIT(var, num)  (var &   (1 << num))

// For use in non-Qt classes
#define TR(text) (QObject::tr(text))


inline bool isShiftDown() {
    return QApplication::keyboardModifiers() & Qt::ShiftModifier;
}

inline bool isCtrlDown() {
    return QApplication::keyboardModifiers() & Qt::ControlModifier;
}

// Returns a string of 'num' in hexadecimal format, padded to 8 digits
inline String hexString(int num) {
    String hex = String::number((uint)num, 16).toUpper();
    if (hex.size() < 8) {
        hex = String(8 - hex.size(), '0') + hex;
    }
    return "0x" + hex;
}

// Helper functions for converting between basic Windows and Qt types
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

inline QColor QColorFromRGB(COLORREF rgb) {
    return QColor(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
}

inline COLORREF RGBFromQColor(const QColor& colour) {
    return RGB(colour.red(), colour.green(), colour.blue());
}

// Converts illegal HTML characters to HTML codes
// TODO: Surely Qt should be able do this...
inline String escapeHtml(String in) {
    String result = in.replace('&', "&amp;");
    result = result.replace('<', "&lt;");
    result = result.replace('>', "&gt;");
    return result;
}

#endif  // BALLISTIC_INCLUDE_H
