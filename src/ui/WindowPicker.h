/////////////////////////////////////////////////////////////////////
// File: WindowPicker.h                                            //
// Date: 17/2/10                                                   //
// Desc: A subclass of QWidget which is used for "picking" a       //
//   window to inspect. It captures mouse input when it is pressed //
//   and changes the cursor to a target. When the mouse button is  //
//   released, it releases mouse capture and gets the window under //
//   the cursor.                                                   //
/////////////////////////////////////////////////////////////////////

#ifndef WINDOW_PICKER_H
#define WINDOW_PICKER_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "ui/HighlightWindow.h"
using namespace inspector;

class WindowPicker : public QWidget {
    Q_OBJECT
private:
    static const int padding = 2; // Padding on each side
    QWidget* owner;               // Main window this belongs to
    QPixmap image;
    bool isPressed;
public:
    HighlightWindow highlighter;

public:
    WindowPicker(QWidget* parent = 0, QWidget* owner = 0);

    QSize minimumSizeHint() const;
signals:
    void windowPicked(Window*);
protected:
    void mouseMoveEvent(QMouseEvent* e);
    void mousePressEvent(QMouseEvent* e);
    void mouseReleaseEvent(QMouseEvent* e);
    void keyPressEvent(QKeyEvent* e);
    void paintEvent(QPaintEvent* e);
private:
    void press();
    void unpress();
};

#endif   // WINDOW_PICKER_H