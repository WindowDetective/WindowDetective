/////////////////////////////////////////////////////////////////////
// File: ListBox.cpp                                               //
// Date: 18/1/11                                                   //
// Desc: Object that represents a list box control.                //
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
#include "ui/property_pages/ListBoxPropertyPage.h"
using namespace inspector;


ListBox::ListBox(HWND handle) :
    Window(handle) {
}

/*------------------------------------------------------------------+
| Returns true if the owner application is responsible for drawing  |
| it's contents. See also ListBox::hasStrings.                      |
+------------------------------------------------------------------*/
bool ListBox::isOwnerDrawn() {
    return TEST_BITS(getStyleBits(), LBS_OWNERDRAWFIXED) || 
           TEST_BITS(getStyleBits(), LBS_OWNERDRAWVARIABLE);
}

/*------------------------------------------------------------------+
| This is only used if it is owner drawn. Returns true if the list  |
| box contains items consisting of strings. Otherwise, the control  |
| draws it's own contents, and we cannot get it's text.             |
+------------------------------------------------------------------*/
bool ListBox::hasStrings() {
    return TEST_BITS(getStyleBits(), LBS_HASSTRINGS);
}

/*------------------------------------------------------------------+
| Return the number of items in the list.                           |
+------------------------------------------------------------------*/
uint ListBox::getNumberOfItems() {
    return sendMessage<uint>(LB_GETCOUNT);
}

// TODO: Check if owner drawn first. If true, this won't work
/*------------------------------------------------------------------+
| Return the list of strings kept in this control.                  |
| If ListBox::hasStrings is false, the list will contain integer    |
| values used by the owner-drawing routine.                         |
+------------------------------------------------------------------*/
QList<String> ListBox::getItems() {
    if (items.isEmpty()) {
        bool isError = false;
        uint errorId = 0;
        WCHAR* buffer = NULL;    // Buffer for each item's text.
        int maxLength = 0;       // Used to determine size of buffer
        bool hasStrings = this->hasStrings();

        for (uint i = 0; i < getNumberOfItems(); i++) {
            int length = sendMessage<int,int,int>(LB_GETTEXTLEN, i, NULL) + 1; // +1 for null terminator

            // Optimization: Expanding memory only when needed rather than
            // allocating and destroying separate memory for each string.
            if (length > maxLength) {
                if (buffer) delete[] buffer;
                buffer = new WCHAR[length];
                maxLength = length;
            }

            length = sendMessage<int,int,LPWSTR>(LB_GETTEXT, i, buffer);
            if (length != LB_ERR) {
                if (hasStrings) {
                    items.append(String::fromWCharArray(buffer, length));
                }
                else {
                    // The string will just be the byte array in hex
                    String str;
                    QTextStream stream(&str);
                    // In this case the buffer size is always the size, in bytes, of a DWORD.
                    // But we will also check the buffer's length just to be sure
                    for (uint i = 0; i < sizeof(DWORD) && i < length * sizeof(WCHAR); i++) {
                        stream << String::number((uint)((byte*)buffer)[i], 16).toUpper()
                               << " ";
                    }
                    items.append(str);
                }
            }
            else {
                isError = true;
                errorId = GetLastError();
            }
        }
        if (buffer) delete[] buffer;

        // If there are any errors, wait until the end to report them
        if (isError) {
            Logger::osWarning(errorId, "Could not get some items from "+getDisplayName());
        }
    }
    return items;
}

/*------------------------------------------------------------------+
| Creates and returns a list of property pages for this object.     |
| Note: The UI window takes ownership of these wigdets.             |
+------------------------------------------------------------------*/
QList<AbstractPropertyPage*> ListBox::makePropertyPages() {
    return Window::makePropertyPages() << new ListBoxPropertyPage(this);
}