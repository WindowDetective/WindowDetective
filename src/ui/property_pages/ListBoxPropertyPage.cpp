/////////////////////////////////////////////////////////////////////
// File: ListBoxPropertyPage.cpp                                   //
// Date: 28/1/11                                                   //
// Desc: The property page for ListBox controls. Displays specific //
//   properties and lists all items of the list box.               //
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

#include "PropertiesWindow.h"
#include "ListBoxPropertyPage.h"
#include "ui/StringFormatter.h"
#include "window_detective/main.h"


ListBoxPropertyPage::ListBoxPropertyPage(ListBox* model, QWidget* parent) :
    AbstractPropertyPage(parent), model(model) {
    setWindowTitle("ListBox");
    setupUi();
}

void ListBoxPropertyPage::setupUi() {
    numberOfItemsWidget = makeValueLabel();
    isOwnerDrawnWidget = makeValueLabel();
    listWidget = new QListWidget(this);

    addRow(tr("Number of Items"), numberOfItemsWidget);
    addRow(tr("Is Owner Drawn"), isOwnerDrawnWidget);
    addSpan(tr("Items"), listWidget);
}

/*------------------------------------------------------------------+
| Updates the data in each property widget.                         |
+------------------------------------------------------------------*/
void ListBoxPropertyPage::updateProperties() {
    numberOfItemsWidget->setText(stringLabel(model->getNumberOfItems()));
    isOwnerDrawnWidget->setText(stringLabel(model->isOwnerDrawn()));

    listWidget->clear();
    QList<String>::const_iterator i;
    for (i = model->getItems().constBegin(); i != model->getItems().constEnd(); i++) {
        new QListWidgetItem(*i, listWidget);
    }
}