/////////////////////////////////////////////////////////////////////
// File: MessagesWindow.h                                          //
// Date: 3/5/10                                                    //
// Desc: Used to display the messages of a window. Typically added //
//   to an MDI area as a child window.                             //
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
    // TODO: Also allow saving to XML file. Either use a custom dialog with radio buttons
    //   for text/xml, or have two extension filters.
    //   Use proper DTD for xml creation, make it similar to the message definitions.
    String fileName = QFileDialog::getSaveFileName(this, tr("Save Messages"),
                        QDir::homePath(), "Text files (*.txt)");
    if (!fileName.isEmpty()) {
        QFile file(fileName);
        if (file.open(QFile::WriteOnly)) {
            MessageHandler::current()->writeMessages(client, &file, FormatText);
        }
    }
}

void MessagesWindow::actionAutoExpand() {
    messageWidget->setAutoExpand(actnAutoExpand->isChecked());
}
