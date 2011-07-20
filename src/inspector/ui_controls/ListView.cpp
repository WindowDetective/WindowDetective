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
#include "ui/property_pages/ListViewPropertyPage.h"
#include "window_detective/StringFormatter.h"
using namespace inspector;


/**************************/
/*** ListViewItem class ***/
/**************************/

ListViewItem::ListViewItem(ListViewItemStruct* itemStruct) {
    this->isSelected = itemStruct->isSelected;
    this->index = itemStruct->index;
    this->text = String::fromWCharArray(itemStruct->text);
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

/*------------------------------------------------------------------+
| Returns the total number of items in this list.                   |
+------------------------------------------------------------------*/
uint ListView::getNumberOfItems() {
    return sendMessage<uint>(LVM_GETITEMCOUNT);
}

/*------------------------------------------------------------------+
| Returns the number of items that can fit vertically in the        |
| visible area of a list-view control when in list or report view.  |
| Only fully visible items are counted.                             |
+------------------------------------------------------------------*/
uint ListView::getNumberOfItemsPerPage() {
    return sendMessage<uint>(LVM_GETCOUNTPERPAGE);
}

/*------------------------------------------------------------------+
| Returns the number of selected items.                             |
+------------------------------------------------------------------*/
uint ListView::getNumberOfSelectedItems() {
    return sendMessage<uint>(LVM_GETSELECTEDCOUNT);
}

/*------------------------------------------------------------------+
| Adds a batch of ListView items to the current collection.         |
| Returns the number of items that were retrieved and added.        |
| <<REMOTE>> Calling LVM_GETITEM must be called from remote process.|
+------------------------------------------------------------------*/
uint ListView::addItemBatch(uint start) {
    // Set up struct to be passed to remote thread
    ListViewItemsStruct itemStruct;
    ZeroMemory(&itemStruct, sizeof(ListViewItemsStruct));
    itemStruct.handle = this->getHandle();
    itemStruct.startIndex = start;
    itemStruct.totalNumber = this->getNumberOfItems();

    // Call the remote function in our hook DLL.
    DWORD result = CallRemoteFunction(handle, "GetListViewItemsRemote",
                              &itemStruct, sizeof(ListViewItemsStruct));

    if (result == S_OK) {
        for (uint i = 0; i < itemStruct.numberRetrieved; i++) {
            items.append(new ListViewItem(&(itemStruct.items[i])));
        }
    }
    else {
        String errorStr = TR("Could not get some list items for ")+getDisplayName();
        if (result == -1) {   // unknown error occurred
            Logger::warning(errorStr);
        }
        else {
            Logger::osWarning(result, errorStr);
        }
    }
    return itemStruct.numberRetrieved;
}

/*------------------------------------------------------------------+
| Returns a list of ListView items.                                 |
+------------------------------------------------------------------*/
QList<ListViewItem*> ListView::getItems() {
    if (items.isEmpty()) {
        uint total = this->getNumberOfItems();
        uint count = 0;
        do {
            uint numberRetrieved = addItemBatch(count);
            if (numberRetrieved == 0) {
                // Not sure what else to do in this case
                return items;
            }
            count += numberRetrieved;
        } while (count < total);
        
    }
    return items;
}

/*------------------------------------------------------------------+
| Creates and returns a list of property pages for this object.     |
| Note: The UI window takes ownership of these wigdets.             |
+------------------------------------------------------------------*/
QList<AbstractPropertyPage*> ListView::makePropertyPages() {
    return Window::makePropertyPages() << new ListViewPropertyPage(this);
}

/*------------------------------------------------------------------+
| Writes an XML representation of this object to the given stream.  |
+------------------------------------------------------------------*/
void ListView::writeContents(QXmlStreamWriter& stream) {
    Window::writeContents(stream);

    stream.writeTextElement("itemsPerPage", stringLabel(getNumberOfItemsPerPage()));
    stream.writeTextElement("selectedItems", stringLabel(getNumberOfSelectedItems()));

    stream.writeStartElement("items");
    stream.writeAttribute("count", stringLabel(getNumberOfItems()));
     QList<ListViewItem*> list = getItems();
     QList<ListViewItem*>::const_iterator i;
     for (i = list.constBegin(); i != list.constEnd(); i++) {
         stream.writeEmptyElement("item");
         stream.writeAttribute("index", stringLabel((*i)->index));
         stream.writeAttribute("text", stringLabel((*i)->text));
         stream.writeAttribute("isSelected", stringLabel((*i)->isSelected));
     }
    stream.writeEndElement();
}
