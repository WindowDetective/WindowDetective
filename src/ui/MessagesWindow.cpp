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
    connect(actnSave, SIGNAL(triggered()), this, SLOT(saveButtonClicked()));
    connect(actnAutoExpand, SIGNAL(triggered()), this, SLOT(autoExpandButtonClicked()));
    connect(actnFilter, SIGNAL(triggered()), this, SLOT(filterButtonClicked()));
    connect(actnHighlight, SIGNAL(triggered()), this, SLOT(highlightButtonClicked()));

    setWindowTitle(tr("Window Messages - %1").arg(model->getDisplayName()));
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
| Opens the Message Filter dialog, on the given tab, then applies   |
| the changes if the user accepts.                                  |
+------------------------------------------------------------------*/
void MessagesWindow::openFilterDialog(int tab) {
    MessageFilterDialog filterDialog(this);
    QStringList allMessages;

    QHash<uint,String>::const_iterator i;
    QHash<uint,String> tempList = Resources::generalMessageNames;
    for (i = tempList.begin(); i != tempList.end(); i++) {
        allMessages.append(*i);
    }
    tempList = getModel()->getWindowClass()->getApplicableMessages();
    for (i = tempList.begin(); i != tempList.end(); i++) {
        allMessages.append(*i);
    }
    filterDialog.setMessages(allMessages, messageWidget->getExcludedMessages());
    filterDialog.setHighlightedMessages(messageWidget->getHighlightedMessages());
    filterDialog.setTabIndex(tab);

    if (filterDialog.exec() == QDialog::Accepted) {    
        messageWidget->setExcludedMessages(filterDialog.getExcludedMessages());
        messageWidget->setHighlightedMessages(filterDialog.getHighlightedMessages());
    }
}

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

void MessagesWindow::saveButtonClicked() {
    String fileName = QFileDialog::getSaveFileName(this, tr("Save Messages"),
                        QDir::homePath(), "XML Files (*.xml);;All Files (*.*)");
    if (fileName.isEmpty()) {
        return;    // User cancelled
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        String msg = tr("Could not open file for writing: \"%1\"").arg(fileName);
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

/*------------------------------------------------------------------+
| Sets the message widget to automatically expand new items.        |
+------------------------------------------------------------------*/
void MessagesWindow::autoExpandButtonClicked() {
    messageWidget->setAutoExpand(actnAutoExpand->isChecked());
}

/*------------------------------------------------------------------+
| Opens the message filter dialog on the filter tab..               |
+------------------------------------------------------------------*/
void MessagesWindow::filterButtonClicked() {
    openFilterDialog(0);
}

/*------------------------------------------------------------------+
| Opens the message filter dialog on the highlight tab.             |
+------------------------------------------------------------------*/
void MessagesWindow::highlightButtonClicked() {
    openFilterDialog(1);
}
