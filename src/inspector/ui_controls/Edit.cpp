/////////////////////////////////////////////////////////////////////
// File: Edit.cpp                                                  //
// Date: 18/1/11                                                   //
// Desc: Object that represents a text edit control.               //
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

#include "inspector/inspector.h"
#include "inspector/WindowManager.h"
#include "window_detective/Logger.h"
#include "ui/property_pages/EditPropertyPage.h"
using namespace inspector;

bool Edit::canUndo() {
    return sendMessage<bool>(EM_CANUNDO);
}

bool Edit::isModified() {
    return sendMessage<bool>(EM_GETMODIFY);
}

bool Edit::isMultiLine() {
    return TEST_BITS(getStyleBits(), ES_MULTILINE);
}

QPoint Edit::getSelectionRange() {
    DWORD startPos, endPos;
    if (sendMessage<int, DWORD*, DWORD*>(EM_GETSEL, &startPos, &endPos) != -1) {
        return QPoint(startPos, endPos);
    }
    else {
        return QPoint();
    }
}

uint Edit::getNumberOfLines() {
    return sendMessage<uint>(EM_GETLINECOUNT);
}

uint Edit::getMaximumCharacters() {
    return sendMessage<uint>(EM_GETLIMITTEXT);
}

/*------------------------------------------------------------------+
| Creates and returns a list of property pages for this object.     |
| Note: The UI window takes ownership of these wigdets.             |
+------------------------------------------------------------------*/
QList<AbstractPropertyPage*> Edit::makePropertyPages() {
    return Window::makePropertyPages() << new EditPropertyPage(this);
}