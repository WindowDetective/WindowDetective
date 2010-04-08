/////////////////////////////////////////////////////////////////////
// File: WindowClassComboBox.cpp                                   //
// Date: 1/4/10                                                    //
// Desc: Special combo box that holds a list of window classes and //
//   provides an auto-complete feature for choosing an item.       //
/////////////////////////////////////////////////////////////////////

#include "WindowClassComboBox.h"

WindowClassComboBox::WindowClassComboBox(QWidget* parent) :
    QComboBox(parent),
    windowClasses() {
}

void WindowClassComboBox::setList(QList<WindowClass*> list) {
    QStringList completerList;
    windowClasses = list;

    foreach (WindowClass* windowClass, list) {
        addItem(windowClass->getIcon(), windowClass->getName());
        completerList << windowClass->getName();
    }

    QCompleter* completer = new QCompleter(completerList, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    completer->setCompletionMode(QCompleter::PopupCompletion);
    setCompleter(completer);
}