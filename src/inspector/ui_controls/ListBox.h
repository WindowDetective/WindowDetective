/////////////////////////////////////////////////////////////////////
// File: ListBox.h                                                 //
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

#ifndef LISTBOX_H
#define LISTBOX_H

namespace inspector {

// TODO: Possible actions
//   * add item (string)
//   * remove item

class ListBox : public Window {
private:
    QList<String> items;

public:
    ListBox(HWND handle);
    ~ListBox() {}

    bool isOwnerDrawn();
    bool hasStrings();
    uint getNumberOfItems();
    QList<String> getItems();

    // Menu and other UI methods
    //virtual QList<QAction> getMenuActions();
    QList<AbstractPropertyPage*> makePropertyPages();
};

};   //namespace inspector

#endif  // LISTBOX_H