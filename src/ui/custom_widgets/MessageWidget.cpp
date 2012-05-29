/////////////////////////////////////////////////////////////////////
// File: MessageWidget.cpp                                         //
// Date: 2010-05-03                                                //
// Desc: Widget for displaying a list of messages. Each message is //
//   actually a tree item whos children are the parameters and     //
//   return value.                                                 //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2012 XTAL256

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


#include "MessageWidget.hpp"

#define AUTO_SCROLL_PADDING   30

MessageWidget::MessageWidget(QWidget *parent) :
    QTreeWidget(parent),
    window(NULL),
    autoExpand(false),
    includeOthers(true),
    messageFilters(),
    highlightedMessages(),
    fMap(), hMap(),
    messageSentIcon(":/img/message_sent.png"),
    messagePostedIcon(":/img/message_posted.png"),
    messageReturnedIcon(":/img/message_returned.png") {
    setColumnCount(2);
    setColumnWidth(1, 25);
    header()->swapSections(0, 1);  // Make the expandable column second
    header()->setVisible(false);
}

MessageWidget::~MessageWidget() {
    MessageHandler::current().removeMessageListener(this);
}

void MessageWidget::listenTo(Window* window) {
    this->window = window;
    MessageHandler::current().addMessageListener(this, window);
}

/*--------------------------------------------------------------------------+
| These functions create tree widget items for each message or struct field |
| (name and data), formatted as a string. Child structs are built           |
| recursively.  Used by the messageAdded function.                          |
+--------------------------------------------------------------------------*/
void addStructField(QTreeWidgetItem* parent, const FieldDefinition& field, byte* data, bool autoExpand) {
    QTreeWidgetItem* item = new QTreeWidgetItem(parent);
    item->setExpanded(autoExpand);
    if (field.getType()->isStruct()) {
        item->setText(0, field.getName());
        StructDefinition* structDefn = static_cast<StructDefinition*>(field.getType());
        for (int i = 0; i < structDefn->numFields(); ++i) {
            const FieldDefinition& subField = structDefn->getField(i);
            byte* subFieldData = data + subField.getOffset();
            addStructField(item, subField, subFieldData, autoExpand);
        }
    }
    else {
        item->setText(0, field.getName() + " = " + field.toString(data));
    }
}
void addStruct(QTreeWidgetItem* parent, const DynamicStruct& ds, bool autoExpand) {
    if (!ds.isNull()) {
        StructDefinition* defn = ds.getDefinition();
        for (int i = 0; i < defn->numFields(); ++i) {
            const FieldDefinition& field = defn->getField(i);
            byte* data = ds.getData() + field.getOffset();
            addStructField(parent, field, data, autoExpand);
        }
    }
}
void addMessageParams(QTreeWidgetItem* parent, WindowMessage* msg, bool autoExpand) {
    const WindowMessageDefn* defn = msg->getDefinition();

    const QList<MessageParameter> params = defn->getParams();
    QList<MessageParameter>::const_iterator i;
    for (i = params.begin(); i != params.end(); ++i) {
        QTreeWidgetItem* paramItem = new QTreeWidgetItem(parent);
        paramItem->setExpanded(autoExpand);
        if (i->getType()->isStruct()) {
            paramItem->setText(0, i->getName());
            addStruct(paramItem, msg->getExtraData(), autoExpand);
        }
        else {
            paramItem->setText(0, i->getName() + " = " + i->toString(msg));
        }
    }
}

/*--------------------------------------------------------------------------+
| Called when a new message has been sent to the window. This adds a new    |
| widget item to the list, with the message's data.                         |
+--------------------------------------------------------------------------*/
void MessageWidget::messageAdded(WindowMessage* msg) {
    // Sanity check - we should only recieve messages sent to the window we are monitoring
    if (this->window != msg->getWindow()) {
        Logger::debug(TR("Message sent to wrong window.\n"
                        "message ID = %1, sent to = %2, supposed to go to = %3")
                        .arg(msg->getId())
                        .arg(this->window->getDisplayName())
                        .arg(msg->getWindow()->getDisplayName()));
        return;
    }

    String msgName = msg->getName();

    // Check if the message should be excluded (filtered).
    if (fMap.contains(msgName)) {
        if (!fMap.value(msgName)) {
            return;
        }
    }
    else if (!includeOthers) {
        return;
    }

    // Get scrollbar position before we add the item
    QScrollBar* sb = verticalScrollBar();
    bool autoScroll = (sb && sb->value() >= sb->maximum()-AUTO_SCROLL_PADDING);

    // Create the tree items and add them to the tree/list
    QTreeWidgetItem* item = new QTreeWidgetItem(this);
    if (msg->isSent())        { item->setIcon(1, messageSentIcon);    }
    else if (msg->isPosted()) { item->setIcon(1, messagePostedIcon);  }
    else if (msg->isReturn()) { item->setIcon(1, messageReturnedIcon);}
    item->setText(0, msgName + " (" + hexString(msg->getId(), 0) + ")");
    item->setExpanded(autoExpand);

    addMessageParams(item, msg, autoExpand);

    if (msg->isReturn()) {
        QTreeWidgetItem* resultItem = new QTreeWidgetItem(item);
        resultItem->setText(0, "returnResult = " + hexString((uint)msg->getReturnValue()));
    }

    // Highlight text/background if any colours are set
    if (hMap.contains(msgName)) {
        QPair<QColor, QColor> colours = hMap.value(msgName);
        item->setForeground(0, QBrush(colours.first));
        item->setBackground(0, QBrush(colours.second));
    }

    // Auto-scroll if necessary
    if (autoScroll) scrollToBottom();
}

void MessageWidget::messageRemoved(WindowMessage* msg) {
    // TODO
}

/*--------------------------------------------------------------------------+
| Sets the list of message filters. A hash map is used                      |
| internally for faster lookups.                                            |
+--------------------------------------------------------------------------*/
void MessageWidget::setMessageFilters(QList<MessageFilter> list) {
    messageFilters = list;
    fMap.clear();

    QList<MessageFilter>::const_iterator i;
    for (i = list.begin(); i != list.end(); ++i) {
        fMap.insert(i->name, i->include);
    }
}


/*--------------------------------------------------------------------------+
| Sets the list of message highlights. A hash map is used                   |
| internally for faster lookups.                                            |
+--------------------------------------------------------------------------*/
void MessageWidget::setHighlightedMessages(QList<MessageHighlight> list) {
    highlightedMessages = list;
    hMap.clear();

    QList<MessageHighlight>::const_iterator i;
    for (i = list.begin(); i != list.end(); ++i) {
        hMap.insert(i->name, qMakePair(i->foregroundColour, i->backgroundColour));
    }
}
