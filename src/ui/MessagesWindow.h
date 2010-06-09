/////////////////////////////////////////////////////////////////////
// File: MessagesWindow.h                                          //
// Date: 3/5/10                                                    //
// Desc: Used to display the messages of a window. Typically added //
//   to an MDI area as a child window.                             //
//   Note: Throughout this class, the term 'client' is used to     //
//   describe the window that this will display info for.          //
/////////////////////////////////////////////////////////////////////

#ifndef MESSAGES_WINDOW_H
#define MESSAGES_WINDOW_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "ui_MessagesWindow.h"
using namespace inspector;

class MessagesWindow : public QMainWindow, private Ui::MessagesWindow {
    Q_OBJECT
private:
    Window* client;
public:
    MessagesWindow(Window* window, QWidget* parent = 0);
    ~MessagesWindow() {}

private slots:
    void actionSave();
    void actionAutoExpand();
};

#endif   // MESSAGES_WINDOW_H