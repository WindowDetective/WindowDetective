/////////////////////////////////////////////////////////////////////
// File: MessagesWindow.h                                          //
// Date: 3/5/10                                                    //
// Desc: Used to display the messages of a window. Typically added //
//   to an MDI area as a child window.                             //
/////////////////////////////////////////////////////////////////////

#include "MessagesWindow.h"
#include "inspector/MessageHandler.h"

MessagesWindow::MessagesWindow(Window* window, QWidget* parent) :
    QMainWindow(parent),
    client(window) {
    setupUi(this);
    Q_ASSERT(window != NULL);
    messageWidget->listenTo(client);

    connect(actnSave, SIGNAL(triggered()), this, SLOT(actionSave()));
    connect(actnAutoExpand, SIGNAL(triggered()), this, SLOT(actionAutoExpand()));
}

void MessagesWindow::actionSave() {
    // TODO:
}

void MessagesWindow::actionAutoExpand() {
    messageWidget->setAutoExpand(actnAutoExpand->isChecked());
}
