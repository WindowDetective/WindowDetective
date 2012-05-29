/////////////////////////////////////////////////////////////////////
// File: CustomComboBoxes.hpp                                      //
// Date: 2010-04-01                                                //
// Desc: Specialized combo boxes that hold a list of objects such  //
//   as windows, window classes and window styles.                 //
//   Each provide an auto-complete feature for choosing an item.   //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2012 XTAL256

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

#ifndef CUSTOM_COMBO_BOXES_H
#define CUSTOM_COMBO_BOXES_H

#include "window_detective/include.h"
#include "inspector/inspector.h"


// TODO: Can some of this be refactored?

class WindowClassComboBox : public QComboBox {
    Q_OBJECT
private:
    QList<WindowClass*> windowClasses;

public:
    WindowClassComboBox(QWidget *parent = 0);
    ~WindowClassComboBox() {}

    WindowClass* currentValue();
    QList<WindowClass*> getList() { return windowClasses; }
    void setList(QList<WindowClass*> list);
};


class WindowStyleComboBox : public QComboBox {
    Q_OBJECT
private:
    QList<WindowStyle*> windowStyles;

public:
    WindowStyleComboBox(QWidget *parent = 0);
    ~WindowStyleComboBox() {}

    WindowStyle* currentValue();
    QList<WindowStyle*> getList() { return windowStyles; }
    void setList(QList<WindowStyle*> list);
};


#endif   // CUSTOM_COMBO_BOXES_H