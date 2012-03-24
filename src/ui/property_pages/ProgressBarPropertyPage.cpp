/////////////////////////////////////////////////////////////////////
// File: ProgressBarPropertyPage.cpp                               //
// Date: 1/8/11                                                    //
// Desc: The property page for ProgressBar controls.               //
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

#include "PropertiesWindow.hpp"
#include "ProgressBarPropertyPage.hpp"
#include "window_detective/StringFormatter.h"


ProgressBarPropertyPage::ProgressBarPropertyPage(ProgressBar* model, QWidget* parent) :
    AbstractPropertyPage(parent), model(model) {
    setWindowTitle("ProgressBar");
}

void ProgressBarPropertyPage::setupUi() {
    addRow(tr("Current Position"), currentPosWidget = makeValueLabel());
    addRow(tr("Step Increment"), stepIncrementWidget = makeValueLabel());
    addRow(tr("Minimum Value"), minimumWidget = makeValueLabel());
    addRow(tr("Maximum Value"), maximumWidget = makeValueLabel());

    if (getOSVersion() >= 600) {
        addRow(tr("State"), stateWidget = makeValueLabel());
    }
    else {
        stateWidget = NULL;
    }
}

/*--------------------------------------------------------------------------+
| Updates the data in each property widget.                                 |
+--------------------------------------------------------------------------*/
void ProgressBarPropertyPage::updateProperties() {
    currentPosWidget->setText(stringLabel(model->getCurrentPosition()));
    stepIncrementWidget->setText(stringLabel(model->getStepIncrement()));
    minimumWidget->setText(stringLabel(model->getMinimum()));
    maximumWidget->setText(stringLabel(model->getMaximum()));
    if (stateWidget) {
        stateWidget->setText(Resources::getConstant("ProgressBarState", model->getState()));
    }
}
