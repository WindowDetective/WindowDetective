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
    window(NULL),
    autoExpand(false),
    excludedMessages(),
    highlightedMessages() {
}

MessageWidget::~MessageWidget() {
    MessageHandler::current()->removeMessageListener(this);
}

void MessageWidget::listenTo(Window* window) {
    this->window = window;
    MessageHandler::current()->addMessageListener(this, window);
}

void MessageWidget::messageAdded(WindowMessage* msg) {
    String msgName = msg->getName(window ? window->getWindowClass() : NULL);
    if (excludedMessages.contains(msgName)) {
        return;    // Filter message
    }

    // Get scrollbar position before we add the item
    QScrollBar* sb = verticalScrollBar();
    bool autoScroll = (sb && sb->value() >= sb->maximum()-AUTO_SCROLL_PADDING);

    // Create the tree items and add them to the tree/list
    QTreeWidgetItem* item = new QTreeWidgetItem(this);
    item->setText(0, msgName);
    item->setExpanded(autoExpand);

    String wParam = "wParam = " + hexString(msg->wParam);
    String lParam = "lParam = " + hexString(msg->lParam);
    new QTreeWidgetItem(item, QStringList(wParam));
    new QTreeWidgetItem(item, QStringList(lParam));

    // Highlight text/background if any colours are set
    if (hMap.contains(msgName)) {
        QPair<QColor, QColor> colours = hMap.value(msgName);
        item->setForeground(0, QBrush(colours.first));
        item->setBackground(0, QBrush(colours.second));
    }

    // Auto-scroll if necessary
    if (autoScroll) scrollToBottom();
}

void MessageWidget::messageReturned(WindowMessage* msg) {
    int numItems = topLevelItemCount();
    if (numItems < 1) return;

    String msgName = msg->getName(window ? window->getWindowClass() : NULL);

    QTreeWidgetItem* item = topLevelItem(numItems - 1);
    if (item->text(0) == msgName) {
        String result = "returnResult = " + hexString(msg->returnValue);
        new QTreeWidgetItem(item, QStringList(result));
    }
}

void MessageWidget::messageRemoved(WindowMessage* msg) {
    // TODO
}

/*------------------------------------------------------------------+
| Sets the list of message highlights. A hash map is used           |
| internally for faster lookups.                                    |
+------------------------------------------------------------------*/
void MessageWidget::setHighlightedMessages(QList<MessageHighlight> list) {
    highlightedMessages = list;
    hMap.clear();

    QList<MessageHighlight>::const_iterator i;
    for (i = list.begin(); i != list.end(); i++) {
        hMap.insert(i->name, qMakePair(i->foregroundColour, i->backgroundColour));
    }
}

/*------------------------------------------------------------------+
| Gets the list of message highlights.                              |
+------------------------------------------------------------------*/
QList<MessageHighlight> MessageWidget::getHighlightedMessages() {
    return highlightedMessages;
}
