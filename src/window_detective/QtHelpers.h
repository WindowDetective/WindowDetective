/////////////////////////////////////////////////////////////////////
// File: QtHelpers.h                                               //
// Date: 15/6/11                                                   //
// Desc: Provides helper functions for working with basic Qt types //
//   These functions:                                              //
//    - convert Windows types to Qt types, and vice versa          //
//    - write a Qt object to an XML stream                         //
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

#ifndef QT_CONVERSIONS_H
#define QT_CONVERSIONS_H

#include "window_detective/include.h"


/*------------------------------------------------------------------+
| Conversion functions                                              |
+------------------------------------------------------------------*/

QColor stringToColour(String string);
String colourToString(QColor colour);

QPoint QPointFromPOINT(const POINT& p);
POINT POINTFromQPoint(const QPoint& p);

QRect QRectFromRECT(const RECT& rect);
RECT RECTFromQRect(const QRect& rect);

QColor QColorFromCOLORREF(COLORREF rgb);
COLORREF COLORREFFromQColor(const QColor& colour);

QDateTime QDateTimeFromSYSTEMTIME(const SYSTEMTIME& st, bool isLocalTime = true);
SYSTEMTIME SYSTEMTIMEFromQDateTime(const QDateTime& dateTime);


/*------------------------------------------------------------------+
| XML functions                                                     |
+------------------------------------------------------------------*/

void writeElement(QXmlStreamWriter& stream, const QRect& rect);
void writeElement(QXmlStreamWriter& stream, const QDateTime& dateTime);


#endif   // QT_CONVERSIONS_H