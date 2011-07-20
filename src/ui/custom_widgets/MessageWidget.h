/////////////////////////////////////////////////////////////////////
// File: MessageWidget.h                                           //
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

#ifndef MESSAGE_WIDGET_H
#define MESSAGE_WIDGET_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "inspector/MessageHandler.h"
using namespace inspector;

struct MessageHighlight {
    String name;
    QColor foregroundColour;
    QColor backgroundColour;
};

class MessageWidget : public QTreeWidget, public WindowMessageListener {
    Q_OBJECT
private:
    bool autoExpand;
    Window* window;
    QList<String> excludedMessages;
    QList<MessageHighlight> highlightedMessages;
    QHash<String, QPair<QColor, QColor> > hMap;

public:
    MessageWidget(QWidget* parent = 0);
    ~MessageWidget();

    void listenTo(Window* window);
    void messageAdded(WindowMessage* msg);
    void messageRemoved(WindowMessage* msg);
    void messageReturned(WindowMessage* msg);
    void setAutoExpand(bool b) { autoExpand = b; }
    bool isAutoExpand() { return autoExpand; }
    void setHighlightedMessages(QList<MessageHighlight> highlights);
    QList<MessageHighlight> getHighlightedMessages();
    void setExcludedMessages(QStringList excluded) { excludedMessages = excluded; }
    QStringList getExcludedMessages() { return excludedMessages; }
};

#endif   // MESSAGE_WIDGET_H