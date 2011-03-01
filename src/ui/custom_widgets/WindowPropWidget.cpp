/////////////////////////////////////////////////////////////////////
// File: WindowPropWidget.cpp                                      //
// Date: 17/2/11                                                   //
// Desc: Displays a list of window user-set properties. These      //
//   properties are set by calling the SetProc API function.       //
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

#include "WindowPropWidget.h"

/*------------------------------------------------------------------+
| WindowPropWidget constructor.                                     |
+------------------------------------------------------------------*/
WindowPropWidget::WindowPropWidget(QWidget* parent) :
    QWidget(parent),
    formLayout(NULL) {
}

/*------------------------------------------------------------------+
| Updates the data in the UI.                                       |
+------------------------------------------------------------------*/
void WindowPropWidget::update() {
    if (formLayout) delete formLayout;
    formLayout = new QFormLayout(this);
    formLayout->setContentsMargins(0, 0, 0, 0);

    QLabel* nameTitle = new QLabel(tr("Name"), this);
    QLabel* dataTitle = new QLabel(tr("Data"), this);
    QFont font;
    font.setBold(true);
    font.setWeight(75);
    nameTitle->setFont(font);   // TODO: Use stylesheet
    dataTitle->setFont(font);
    formLayout->addRow(nameTitle, dataTitle);

    WindowPropList::const_iterator i;
    for (i = model.constBegin(); i != model.constEnd(); i++) {
        QLabel* nameLabel = new QLabel((*i).name, this);
        QLabel* dataLabel = new QLabel(hexString((int)(*i).data), this);
        nameLabel->setCursor(QCursor(Qt::IBeamCursor));
        nameLabel->setTextFormat(Qt::PlainText);
        nameLabel->setTextInteractionFlags(Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard);
        dataLabel->setCursor(QCursor(Qt::IBeamCursor));
        dataLabel->setTextFormat(Qt::PlainText);
        dataLabel->setTextInteractionFlags(Qt::TextBrowserInteraction|Qt::TextSelectableByKeyboard);
        formLayout->addRow(nameLabel, dataLabel);
    }
}
