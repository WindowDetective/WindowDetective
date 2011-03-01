/////////////////////////////////////////////////////////////////////
// File: PropertiesWindow.cpp                                      //
// Date: 23/3/10                                                   //
// Desc: Used to display the properties of a window. Typically     //
//   added to an MDI area as a child window.                       //
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
#include "WindowClassPropertyPage.h"

PropertiesWindow::PropertiesWindow(Window* window, QWidget* parent) :
    QMainWindow(parent), model(window),
    pages(), hasInitialized() {
    setupUi(this);
    Q_ASSERT(window != NULL);

    connect(actnLocate, SIGNAL(triggered()), this, SLOT(locateActionTriggered()));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabPageChanged(int)));

    setWindowTitle(tr("Window Properties - ")+model->getDisplayName());
    createPages();
    tabPageChanged(0);  // Initialize first page only
}

/*------------------------------------------------------------------+
| Creates the property pages and adds them to tabs.                 |
+------------------------------------------------------------------*/
void PropertiesWindow::createPages() {
    QList<AbstractPropertyPage*> pages = model->makePropertyPages();

    QList<AbstractPropertyPage*>::const_iterator i;
    for (i = pages.constBegin(); i != pages.constEnd(); i++) {
        addPropertyPage(*i, (*i)->windowTitle());
    }
    addPropertyPage(new WindowClassPropertyPage(model->getWindowClass()), "Window Class");
}

/*------------------------------------------------------------------+
| Adds the given widget as a tab with the given title.              |
+------------------------------------------------------------------*/
void PropertiesWindow::addPropertyPage(AbstractPropertyPage* page, String title) {
    QScrollArea* scrollArea = new QScrollArea(this);
    scrollArea->setWidget(page);
    scrollArea->setWidgetResizable(true);
    tabWidget->addTab(scrollArea, title);
    pages.append(page);
    hasInitialized.append(false);
}

/*------------------------------------------------------------------+
| This just forwards the signal on with this client window.         |
+------------------------------------------------------------------*/
void PropertiesWindow::locateActionTriggered() {
    emit locateWindow(model);
}

/*------------------------------------------------------------------+
| The tab page has just changed. If the newly selected page has not |
| been initialized yet, do it now.                                  |
+------------------------------------------------------------------*/
void PropertiesWindow::tabPageChanged(int index) {
    if (index < hasInitialized.size()) {
        if (!hasInitialized.at(index)) {
            pages.at(index)->updateProperties();
            hasInitialized[index] = true;
        }
    }
}

/*------------------------------------------------------------------+
| The window has changed, set the properties again.                 |
+------------------------------------------------------------------*/
void PropertiesWindow::update() {
    // Set initialized flag to false for each page.
    QList<bool>::iterator i;
    for (i = hasInitialized.begin(); i != hasInitialized.end(); i++) {
        *i = false;
    }

    // Now refresh the current page
    tabPageChanged(tabWidget->currentIndex());
}
