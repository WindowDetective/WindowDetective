/////////////////////////////////////////////////////////////////////
// File: MessageWidget.cpp                                         //
// Date: 3/5/10                                                    //
// Desc: Widget for displaying a list of messages. Each message is //
//   actually a tree item whos children are the parameters and     //
//   return value.                                                 //
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


#include "MessageWidget.h"

#define AUTO_SCROLL_PADDING   30

MessageWidget::MessageWidget(QWidget *parent) :
    QTreeWidget(parent),
    autoExpand(false) {
}

MessageWidget::~MessageWidget() {
    MessageHandler::current()->removeMessageListener(this);
}

void MessageWidget::listenTo(Window* window) {
    MessageHandler::current()->addMessageListener(this, window);
}

void MessageWidget::messageAdded(WindowMessage* msg) {
    // Get scrollbar position before we add the item
    QScrollBar* sb = verticalScrollBar();
    bool autoScroll = (sb && sb->value() >= sb->maximum()-AUTO_SCROLL_PADDING);

    // Create the tree items and add them to the tree/list
    QTreeWidgetItem* item = new QTreeWidgetItem(this);
    item->setText(0, msg->getName());
    item->setExpanded(autoExpand);

    String wParam = "wParam = " + hexString(msg->wParam);
    String lParam = "lParam = " + hexString(msg->lParam);
    new QTreeWidgetItem(item, QStringList(wParam));
    new QTreeWidgetItem(item, QStringList(lParam));

    // Auto-scroll if necessary
    if (autoScroll) scrollToBottom();
}

void MessageWidget::messageReturned(WindowMessage* msg) {
    int numItems = topLevelItemCount();
    if (numItems < 1) return;

    QTreeWidgetItem* item = topLevelItem(numItems - 1);
    if (item->text(0) == msg->getName()) {
        String result = "returnResult = " + hexString(msg->returnValue);
        new QTreeWidgetItem(item, QStringList(result));
    }
}

void MessageWidget::messageRemoved(WindowMessage* msg) {
    // TODO
}