/////////////////////////////////////////////////////////////////////
// File: WindowPicker.cpp                                          //
// Date: 17/2/10                                                   //
// Desc: A subclass of QWidget which is used for "picking" a       //
//   window to inspect. It captures mouse input when it is pressed //
//   and changes the cursor to a target. When the mouse button is  //
//   released, it releases mouse capture and gets the window under //
//   the cursor.                                                   //
/////////////////////////////////////////////////////////////////////

#include "window_detective/main.h"
#include "inspector/WindowManager.h"
#include "window_detective/Settings.h"
#include "WindowPicker.h"


WindowPicker::WindowPicker(QWidget* parent, QWidget* owner) :
    QWidget(parent),
    owner(owner),
    highlighter(true),
    isPressed(false) {
    image.load(":/img/picker.png");
    highlighter.create();
}

QSize WindowPicker::minimumSizeHint() const {
    return QSize(image.height() + (padding * 2) /*+ width of text*/, 24);
}

void WindowPicker::press() {
    isPressed = true;
    update();
    showPickerCursor();
    /* FIXME: When window is hidden, mouse doesn't seem to be grabbed!
    if (owner && Settings::hideWhilePicking)
        owner->hide();*/
    //grabMouse();  Mouse already grabbed by QWidget i think
}

void WindowPicker::unpress() {
    isPressed = false;
    releaseMouse();
    restoreCursor();
    highlighter.hide();
    /* FIXME: When window is hidden, mouse doesn't seem to be grabbed!
    if (owner && Settings::hideWhilePicking)
        owner->show();*/
    update();
}


/**********************/
/*** Event handlers ***/
/**********************/

void WindowPicker::mouseMoveEvent(QMouseEvent* e) {
    // Note: Move events are only generated when a mouse button is down
    QPoint p = QCursor::pos();
    Window* windowUnderCursor = WindowManager::getCurrent()->getWindowAt(p);
    if (windowUnderCursor)
        highlighter.highlight(windowUnderCursor);
    e->accept();
}

void WindowPicker::mousePressEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
        press();
        e->accept();
    }
}

void WindowPicker::mouseReleaseEvent(QMouseEvent* e) {
    unpress();
    e->accept();
    QPoint p = QCursor::pos();
    Window* windowUnderCursor = WindowManager::getCurrent()->getWindowAt(p);
    if (windowUnderCursor)
        emit windowPicked(windowUnderCursor);
};

void WindowPicker::keyPressEvent(QKeyEvent* e) {
    // TODO: Get this working
    if (e->key() == Qt::Key_Escape) {
        unpress();
        e->accept();
    }
    else {
        e->ignore();
    }
}

void WindowPicker::paintEvent(QPaintEvent* /*e*/) {
    QPainter painter(this);

    if (!isPressed) {
        int imgTop = (this->width() - image.width())/2;
        painter.drawPixmap(2, imgTop, image);
    }
    // draw text "Pick window"
}