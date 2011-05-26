/////////////////////////////////////////////////////////////////////
// File: GenericPropertyPage.cpp                                   //
// Date: 25/1/11                                                   //
// Desc: The property page for Window objects. Displays properties //
//    common to all windows.                                       //
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
#include "GenericPropertyPage.h"
#include "ui/StringFormatter.h"
#include "window_detective/main.h"


GenericPropertyPage::GenericPropertyPage(Window* model, QWidget* parent) :
    AbstractPropertyPage(parent), model(model) {
    setWindowTitle("Window");
    setupUi();
}

void GenericPropertyPage::setupUi() {
    addRow(tr("Window Text"), windowTextWidget = makeValueLabel());
    addRow(tr("Handle"), handleWidget = makeValueLabel());
    if (model->getParent()) {
        addRow(tr("Parent Handle"), parentHandleWidget = makeValueLabel());
    }
    else {
        parentHandleWidget = NULL;
    }
    addRow(tr("Dimensions"), dimensionsWidget = makeValueLabel());
    addRow(tr("Position"), positionWidget = makeValueLabel());
    addRow(tr("Size"), sizeWidget = makeValueLabel());
    if (model->isChild()) {
        addRow(tr("Relative Dimensions"), relativeDimensionsWidget = makeValueLabel());
    }
    else {
        relativeDimensionsWidget = NULL;
    }
    addRow(tr("Client Dimensions"), clientDimensionsWidget = makeValueLabel());
    addRow(tr("Style bits"), styleBitsWidget = makeValueLabel());
    addRow(tr("Styles"), stylesWidget = makeValueLabel());
    addRow(tr("Extended Style bits"), exStyleBitsWidget = makeValueLabel());
    addRow(tr("Extended Styles"), exStylesWidget = makeValueLabel());
    addRow(tr("Font"), fontWidget = new FontPropertyWidget(this));
    addRow(tr("Window Props"), windowPropsWidget = new WindowPropWidget(this));
    addRow(tr("Owner Process"), ownerProcessWidget = makeValueLabel());
    addRow(tr("Owner Process ID"), ownerProcessIDWidget = makeValueLabel());
    addRow(tr("Owner Thread ID"), ownerThreadIDWidget = makeValueLabel());
}

/*------------------------------------------------------------------+
| Updates the data in each property widget.                         |
+------------------------------------------------------------------*/
void GenericPropertyPage::updateProperties() {
    model->updateExtraInfo();

    windowTextWidget->setText(stringLabel(model->getText()));
    handleWidget->setText(stringLabel(model->getHandle()));
    if (parentHandleWidget) {
        parentHandleWidget->setText(stringLabel(model->getParent()->getHandle()));
    }
    dimensionsWidget->setText(stringLabel(model->getDimensions()));
    positionWidget->setText(stringLabel(model->getPosition()));
    sizeWidget->setText(stringLabel(model->getSize()));
    if (relativeDimensionsWidget) {
        relativeDimensionsWidget->setText(stringLabel(model->getRelativeDimensions()));
    }
    clientDimensionsWidget->setText(stringLabel(model->getClientDimensions()));
    styleBitsWidget->setText(hexString(model->getStyleBits()));
    stylesWidget->setText(stringLabel(model->getStandardStyles()));
    exStyleBitsWidget->setText(hexString(model->getExStyleBits()));
    exStylesWidget->setText(stringLabel(model->getExtendedStyles()));
    fontWidget->setModel(model->getFont());
    windowPropsWidget->setModel(model->getProps());
    ownerProcessWidget->setText(stringLabel(model->getProcess()->getFilePath()));
    ownerProcessIDWidget->setText(stringLabel(model->getProcessId()));
    ownerThreadIDWidget->setText(stringLabel(model->getThreadId()));
}
