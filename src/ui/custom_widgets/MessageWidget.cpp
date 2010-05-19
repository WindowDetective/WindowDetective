/////////////////////////////////////////////////////////////////////
// File: MessageWidget.cpp                                         //
// Date: 3/5/10                                                    //
// Desc: Widget for displaying a list of messages. Each message is //
//   actually a tree item whos children are the parameters and     //
//   return value.                                                 //
/////////////////////////////////////////////////////////////////////


#include "MessageWidget.h"

MessageWidget::MessageWidget(QWidget *parent) :
    QTreeWidget(parent) {
}

MessageWidget::~MessageWidget() {
    MessageHandler::current()->removeMessageListener(this);
}

void MessageWidget::listenTo(Window* window) {
    MessageHandler::current()->addMessageListener(this, window);
}

void MessageWidget::messageAdded(WindowMessage* msg) {
    QTreeWidgetItem* item = new QTreeWidgetItem(this);
    item->setText(0, msg->getName());

    String wParam = "wParam = " + hexString(msg->wParam);
    String lParam = "lParam = " + hexString(msg->lParam);
    String result = "returnResult = " + hexString(msg->returnValue);
    new QTreeWidgetItem(item, QStringList(wParam));
    new QTreeWidgetItem(item, QStringList(lParam));
    new QTreeWidgetItem(item, QStringList(result));
}

void MessageWidget::messageRemoved(WindowMessage* msg) {
    // TODO
}
