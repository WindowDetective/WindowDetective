/////////////////////////////////////////////////////////////////////
// File: MessagesWindow.h                                          //
// Date: 3/5/10                                                    //
// Desc: Used to display the messages of a window. Typically added //
//   to an MDI area as a child window.                             //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2011 XTAL256

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
    model(window) {
    setupUi(this);
    Q_ASSERT(model != NULL);
    messageWidget->listenTo(model);
    messageWidget->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(messageWidget, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showContextMenu(const QPoint&)));
    connect(actnLocate, SIGNAL(triggered()), this, SLOT(locateActionTriggered()));
    connect(actnSave, SIGNAL(triggered()), this, SLOT(saveToFile()));
    connect(actnAutoExpand, SIGNAL(triggered()), this, SLOT(autoExpand()));

    setWindowTitle(tr("Window Messages - ")+model->getDisplayName());
}

void MessagesWindow::setModel(Window* model) {
    this->model = model;
    // TODO: Need to remove listener for old model and add new one
}

/*------------------------------------------------------------------+
| Constructs a menu with actions for the given items.               |
+------------------------------------------------------------------*/
//QMenu MessagesWindow::makeContextMenu(selected items) {
//     TODO
//}

/*------------------------------------------------------------------+
| Displays the context menu for the selected item/s.                |
+------------------------------------------------------------------*/
//void MessagesWindow::showContextMenu(const QPoint& /*unused*/) {
//    QMenu menu = makeContextMenu(/*selected items*/);
//    QAction* action = menu.exec(QCursor::pos());
//    if (!action) return;   // User cancelled
//
//    // TODO
//}

/*------------------------------------------------------------------+
| This just forwards the signal on with the model.                  |
+------------------------------------------------------------------*/
void MessagesWindow::locateActionTriggered() {
    emit locateWindow(model);
}

void MessagesWindow::saveToFile() {
    String fileName = QFileDialog::getSaveFileName(this, tr("Save Messages"),
                        QDir::homePath(), "XML Files (*.xml);;All Files (*.*)");
    if (fileName.isEmpty()) {
        return;    // User cancelled
    }
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        String msg = tr("Could not open file for writing.")+"\n\""+fileName+"\"";
        QMessageBox::warning(this, tr("Save Window Messages"), msg);
        Logger::error(msg);
        return;
    }

    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.setAutoFormattingIndent(4);  // 4 spaces
    stream.writeStartDocument();
    MessageHandler::current()->writeMessagesToXml(model, stream);
    stream.writeEndDocument();
}

void MessagesWindow::autoExpand() {
    messageWidget->setAutoExpand(actnAutoExpand->isChecked());
}
