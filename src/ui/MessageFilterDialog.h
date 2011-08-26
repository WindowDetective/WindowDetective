/////////////////////////////////////////////////////////////////////
// File: MessageFilterDialog.h                                     //
// Date: 23/6/11                                                   //
// Desc: Dialog window used to manage message filters for a        //
//   messages window. The filters can be inclusions/exclusions or  //
//   highlighting of particular messages.                          //
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

#ifndef MESSAGE_FILTER_DIALOG_H
#define MESSAGE_FILTER_DIALOG_H

#include "window_detective/include.h"
#include "ui/forms/ui_MessageFilterDialog.h"
#include "ui/custom_widgets/MessageWidget.h" // For MessageHighlight definition.


class MessageFilterDialog : public QDialog, private Ui::MessageFilterDialog {
    Q_OBJECT
private:
    QStringListModel allMessagesModel;    // Used for combo-boxes

public:
    MessageFilterDialog(QWidget* parent = 0);
    ~MessageFilterDialog() {}

    void setTabIndex(int i) { tabWidget->setCurrentIndex(i); }
    void setMessages(QStringList all, QStringList excluded);
    void setHighlightedMessages(QList<MessageHighlight> highlights);
    QStringListModel* getIncludedMessagesModel();
    QStringListModel* getExcludedMessagesModel();
    QStringList getIncludedMessages();
    QStringList getExcludedMessages();
    QList<MessageHighlight> getHighlightedMessages();
private:
    void moveSelectedItems(QListView* fromView, QListView* toView);
    void addHighlight(String messageName, QColor foreColour, QColor backColour);
private slots:
    void excludeButtonClicked();
    void includeButtonClicked();
    void addHighlightButtonClicked();
    void removeHighlightButtonClicked();
    void highlightCellDoubleClicked(int row, int column);
    void highlightItemSelectionChanged();
};

#endif   // MESSAGE_FILTER_DIALOG_H