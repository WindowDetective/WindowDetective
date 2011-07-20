/////////////////////////////////////////////////////////////////////
// File: DateTimePickerPropertyPage.cpp                            //
// Date: 10/6/11                                                   //
// Desc: The property page for DateTimePicker controls.            //
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
#include "DateTimePickerPropertyPage.h"
#include "window_detective/StringFormatter.h"
#include "window_detective/main.h"


DateTimePickerPropertyPage::DateTimePickerPropertyPage(DateTimePicker* model, QWidget* parent) :
    AbstractPropertyPage(parent), model(model) {
    setWindowTitle("DateTimePicker");
    setupUi();
}

void DateTimePickerPropertyPage::setupUi() {
    selectedTimeWidget = makeValueLabel();
    minimumTimeWidget = makeValueLabel();
    maximumTimeWidget = makeValueLabel();

    addRow(tr("Selected Date/Time"), selectedTimeWidget);
    addRow(tr("Minimum Allowable Date/Time"), minimumTimeWidget);
    addRow(tr("Maximum Allowable Date/Time"), maximumTimeWidget);
}

/*------------------------------------------------------------------+
| Updates the data in each property widget.                         |
+------------------------------------------------------------------*/
void DateTimePickerPropertyPage::updateProperties() {
    model->getRemoteInfo();  // We know we will need this info, so just get it now

    QDateTime selectedDateTime = model->getSelected();
    if (selectedDateTime.isNull()) {
        selectedTimeWidget->setText(tr("none"));
    }
    else {
        selectedTimeWidget->setText(stringLabel(selectedDateTime));
    }

    QDateTime minimumDateTime = model->getMinimum();
    if (minimumDateTime.isNull()) {
        minimumTimeWidget->setText(tr("none"));
    }
    else {
        minimumTimeWidget->setText(stringLabel(minimumDateTime));
    }

    QDateTime maximumDateTime = model->getMaximum();
    if (maximumDateTime.isNull()) {
        maximumTimeWidget->setText(tr("none"));
    }
    else {
        maximumTimeWidget->setText(stringLabel(maximumDateTime));
    }
}
