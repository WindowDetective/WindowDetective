/////////////////////////////////////////////////////////////////////
// File: WindowClassComboBox.h                                     //
// Date: 1/4/10                                                    //
// Desc: Special combo box that holds a list of window classes and //
//   provides an auto-complete feature for choosing an item.       //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010 XTAL256

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

#ifndef WINDOW_CLASS_COMBO_BOX_H
#define WINDOW_CLASS_COMBO_BOX_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
using namespace inspector;

class WindowClassComboBox : public QComboBox {
    Q_OBJECT
private:
    QList<WindowClass*> windowClasses;

public:
    WindowClassComboBox(QWidget *parent = 0);
    ~WindowClassComboBox() {}

    QList<WindowClass*> getList() { return windowClasses; }
    void setList(QList<WindowClass*> list);
};

#endif   // WINDOW_CLASS_COMBO_BOX_H