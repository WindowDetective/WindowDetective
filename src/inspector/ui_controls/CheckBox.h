/////////////////////////////////////////////////////////////////////
// File: CheckBox.h                                                //
// Date: 18/1/11                                                   //
// Desc: Object that represents a check box control (Button with   //
//   BS_CHECKBOX or BS_AUTOCHECKBOX style.                         //
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

#ifndef CHECKBOX_H
#define CHECKBOX_H

namespace inspector {

// TODO: Possible actions
//   * check/uncheck  (may be the same as Button click)

class CheckBox : public Window {
public:
    CheckBox(HWND handle) : Window(handle) {}

    String getClassName();
    const QIcon getIcon();
};

};   //namespace inspector

#endif  // CHECKBOX_H