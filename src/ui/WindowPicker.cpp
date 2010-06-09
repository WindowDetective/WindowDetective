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
    setAttribute(Qt::WA_NativeWindow);
    pickerText = tr("Pick window");
    image.load(":/img/picker.png");
    highlighter.create();
}

QSize WindowPicker::minimumSizeHint() const {
    QPainter painter(const_cast<WindowPicker*>(this));
    QRect textRect = painter.fontMetrics().boundingRect(pickerText);

    // Width: padding both sides, image and text with 2x padding in between
    // Height: 24, unless text hight is bigger
    return QSize(image.width() + (padding * 4) + textRect.width(),
                 qMax(textRect.height(), 24));
}


Window* windowUnderCursor() {
    QPoint p = QCursor::pos();
    return WindowManager::current()->getWindowAt(p);
}


/**********************/
/*** Event handlers ***/
/**********************/

void WindowPicker::mousePressed() {
    isPressed = true;
    update();
    showPickerCursor();
    if (owner && Settings::hideWhilePicking)
        owner->hide();
    SetCapture(this->winId());
}

void WindowPicker::mouseReleased() {
    isPressed = false;
    ReleaseCapture();
    restoreCursor();
    highlighter.hide();
    if (owner && Settings::hideWhilePicking)
        owner->show();
    update();
}

/*------------------------------------------------------------------+
| Handling native Windows messages gives us more control,           |
| especially with capturing the mouse, somthing that Qt does        |
| automatically in it's own events.                                 |
+------------------------------------------------------------------*/
bool WindowPicker::winEvent(MSG* msg, long* result) {
    switch (msg->message) {
      case WM_LBUTTONDOWN: {
          mousePressed();
          *result = 0;
          return true;
      }
      case WM_LBUTTONUP: {
          if (GetCapture() == winId()) {
              // Get window before showing main window, incase it's under it
              Window* wnd = windowUnderCursor();
              mouseReleased();
              if (wnd) emit windowPicked(wnd);
              *result = 0;
              return true;
          }
      }
      case WM_MOUSEMOVE: {
          if (isPressed && GetCapture() == winId()) {
              Window* wnd = windowUnderCursor();
              if (wnd) highlighter.highlight(wnd);
              *result = 0;
              return true;
          }
      }
      /* TODO: Get this working. This message doesn't even seem to be sent to us
          Also try WM_CAPTURECHANGED, it's supposed to be better
      case WM_CANCELMODE: {
          // User cancelled mouse capture (usually by pressing Esc)
          if (GetCapture() == winId()) {
              mouseReleased();
              *result = 0;
              return true;
          }
      }*/
    }
    return false;
}

void WindowPicker::paintEvent(QPaintEvent* /*e*/) {
    QPainter painter(this);
    QRect textRect = painter.fontMetrics().boundingRect(pickerText);

    if (!isPressed) {
        int imgTop = (this->height() - image.height())/2;
        painter.drawPixmap(2, imgTop, image);
    }
    const int textLeft = image.width() + (padding * 3);
    painter.drawText(textLeft, 0, textRect.width(), this->height(),
                     Qt::AlignVCenter, pickerText);
}