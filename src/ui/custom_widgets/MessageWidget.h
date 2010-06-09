/////////////////////////////////////////////////////////////////////
// File: MessageWidget.h                                           //
// Date: 3/5/10                                                    //
// Desc: Widget for displaying a list of messages. Each message is //
//   actually a tree item whos children are the parameters and     //
//   return value.                                                 //
/////////////////////////////////////////////////////////////////////

#ifndef MESSAGE_WIDGET_H
#define MESSAGE_WIDGET_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "inspector/MessageHandler.h"
using namespace inspector;

class MessageWidget : public QTreeWidget, public WindowMessageListener {
    Q_OBJECT
private:
    bool autoExpand;
public:
    MessageWidget(QWidget *parent = 0);
    ~MessageWidget();

    void listenTo(Window* window);
    void messageAdded(WindowMessage* msg);
    void messageRemoved(WindowMessage* msg);
    void messageReturned(WindowMessage* msg);
    void setAutoExpand(bool b) { autoExpand = b; }
    bool isAutoExpand() { return autoExpand; }
};

#endif   // MESSAGE_WIDGET_H