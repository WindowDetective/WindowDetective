/////////////////////////////////////////////////////////////////////
// File: WindowClassPropertyPage.cpp                               //
// Date: 27/1/11                                                   //
// Desc: The property page for window classes.                     //
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
#include "WindowClassPropertyPage.h"
#include "ui/StringFormatter.h"
#include "window_detective/main.h"


WindowClassPropertyPage::WindowClassPropertyPage(WindowClass* model, QWidget* parent) :
    AbstractPropertyPage(parent), model(model) {
    setupUi();
}

void WindowClassPropertyPage::setupUi() {
    addRow(tr("Class Name"), classNameWidget = makeValueLabel());
    addRow(tr("Class Extra Bytes"), classExtraBytesWidget = makeValueLabel());
    addRow(tr("Window Extra Bytes"), windowExtraBytesWidget = makeValueLabel());
    addRow(tr("Background Brush"), backgroundBrushWidget = new BrushPropertyWidget(this));
}

/*------------------------------------------------------------------+
| Updates the data in each property widget.                         |
+------------------------------------------------------------------*/
void WindowClassPropertyPage::updateProperties() {
    classNameWidget->setText(stringLabel(model->getDisplayName()));
    classExtraBytesWidget->setText(stringLabel(model->getClassExtraBytes()));
    windowExtraBytesWidget->setText(stringLabel(model->getWindowExtraBytes()));
    backgroundBrushWidget->setModel(model->getBackgroundBrush());
}
