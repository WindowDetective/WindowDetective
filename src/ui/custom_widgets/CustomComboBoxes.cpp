/////////////////////////////////////////////////////////////////////
// File: CustomComboBoxes.cpp                                      //
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

#include "CustomComboBoxes.hpp"


/*********************************/
/*** WindowClassComboBox class ***/
/*********************************/

WindowClassComboBox::WindowClassComboBox(QWidget* parent) :
    QComboBox(parent),
    windowClasses() {
    setEditable(true);
    view()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

WindowClass* WindowClassComboBox::currentValue() {
    int index = this->currentIndex();
    return index == -1 ? NULL : windowClasses.at(index);
}

void WindowClassComboBox::setList(QList<WindowClass*> list) {
    QStringList completerList;
    windowClasses = list;

    // TODO: Selecting an item from completer list does not update icon or current index
    clear();
    foreach (WindowClass* windowClass, list) {
        addItem(windowClass->getIcon(), windowClass->getDisplayName());
        completerList << windowClass->getDisplayName();
    }

    QCompleter* completer = new QCompleter(completerList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    setCompleter(completer);
}


/*************************/
/*** WindowStyle class ***/
/*************************/

WindowStyleComboBox::WindowStyleComboBox(QWidget* parent) :
    QComboBox(parent),
    windowStyles() {
    setEditable(true);
    view()->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
}

WindowStyle* WindowStyleComboBox::currentValue() {
    int index = this->currentIndex();
    return index == -1 ? NULL : windowStyles.at(index);
}

void WindowStyleComboBox::setList(QList<WindowStyle*> list) {
    QStringList completerList;
    windowStyles = list;

    clear();
    foreach (WindowStyle* style, list) {
        addItem(style->getName());
        completerList << style->getName();
    }

    QCompleter* completer = new QCompleter(completerList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    setCompleter(completer);
}