/////////////////////////////////////////////////////////////////////
// File: WindowClassComboBox.cpp                                   //
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

#include "WindowClassComboBox.h"

WindowClassComboBox::WindowClassComboBox(QWidget* parent) :
    QComboBox(parent),
    windowClasses() {
}

void WindowClassComboBox::setList(QList<WindowClass*> list) {
    QStringList completerList;
    windowClasses = list;

    clear();  // remove any previous items then add these
    foreach (WindowClass* windowClass, list) {
        addItem(windowClass->getIcon(), windowClass->getName());
        completerList << windowClass->getName();
    }

    QCompleter* completer = new QCompleter(completerList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    setCompleter(completer);
}