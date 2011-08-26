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
#include <QtGui/QtGui>
#include <QtXml/QtXml>
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
| Returns a string of num in hexadecimal format, padded to 8 digits |
+------------------------------------------------------------------*/
static String hexString(uint num) {
    String hex = String::number(num, 16).toUpper();
    if (hex.size() < 8) {
        hex = String(8 - hex.size(), '0') + hex;
    }
    return "0x" + hex;
}

/*------------------------------------------------------------------+
| Returns an integer indicating the version of the operating        |
| system this application is running on. XP is 501.                 |
+------------------------------------------------------------------*/
static int getOSVersion() {
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&info);
    return (info.dwMajorVersion * 100) + info.dwMinorVersion;
}


#endif  // BALLISTIC_INCLUDE_H
