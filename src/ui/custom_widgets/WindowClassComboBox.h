/////////////////////////////////////////////////////////////////////
// File: WindowClassComboBox.h                                     //
// Date: 1/4/10                                                    //
// Desc: Special combo box that holds a list of window classes and //
//   provides an auto-complete feature for choosing an item.       //
/////////////////////////////////////////////////////////////////////

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