/////////////////////////////////////////////////////////////////////
// File: ListView.cpp                                              //
// Date: 12/1/11                                                   //
// Desc: Object that represents a list view control (SysListView32)//
//    and it's associated items (LVITEM).                          //
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
using namespace inspector;


/**************************/
/*** ListViewItem class ***/
/**************************/

ListViewItem::ListViewItem(LVITEMW* itemStruct) {
    itemIndex = itemStruct->iItem;
    text = String::fromWCharArray(itemStruct->pszText);
}


/**********************/
/*** ListView class ***/
/**********************/

ListView::ListView(HWND handle) : 
    Window(handle), items() {
}

ListView::~ListView() {
    QList<ListViewItem*>::iterator i;
    for (i = items.begin(); i != items.end(); i++) {
        delete *i;
    }
}

int ListView::getNumberOfItems() {
    return sendMessage<uint>(LVM_GETITEMCOUNT);
}

/*------------------------------------------------------------------+
| Returns a list of ListView items.                                 |
+------------------------------------------------------------------*/
QList<ListViewItem*> ListView::getItems() {
    if (items.isEmpty()) {
        /*LVITEMW itemStruct = { 0 };
        WCHAR textBuffer[1024];
        bool isError = false;
        uint errorId = 0;

        for (int i = 0; i < getNumberOfItems(); i++) {
            // Indicate what data we want to be returned
            itemStruct.iItem = i;
            itemStruct.iSubItem = 0;
            itemStruct.mask = LVIF_TEXT;  // TODO: More, or all
            itemStruct.pszText = (LPWSTR)&textBuffer;
            itemStruct.cchTextMax = sizeof(textBuffer);

            // The struct will be filled with the requested data
            bool result = sendMessage<bool,int,LVITEMW*>(LVM_GETITEM, 0, &itemStruct);
            if (result) {
                items.append(new ListViewItem(&itemStruct));
            }
            else {
                isError = true;
                errorId = GetLastError();
            }
        }
        // If there are any errors, wait until the end to report them
        if (isError) {
            Logger::osWarning(errorId, "Could not get some items from "+getDisplayName());
        }*/
    }
    return items;
}