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

/*------------------------------------------------------------------+
| Constructor.                                                      |
+------------------------------------------------------------------*/
PropertiesWindow::PropertiesWindow(Window* window, QWidget* parent) :
    QMainWindow(parent), model(window),
    pages(), hasInitialized() {
    setupUi(this);
    Q_ASSERT(window != NULL);

    connect(actnLocate, SIGNAL(triggered()), this, SLOT(locateActionTriggered()));
    connect(actnSave, SIGNAL(triggered()), this, SLOT(saveToFile()));
    connect(tabWidget, SIGNAL(currentChanged(int)), this, SLOT(tabPageChanged(int)));

    setWindowTitle(tr("Window Properties - ")+model->getDisplayName());
    createPages();
    tabPageChanged(0);  // Initialize first page only
}

/*------------------------------------------------------------------+
| Creates the property pages and adds them to tabs.                 |
+------------------------------------------------------------------*/
void PropertiesWindow::createPages() {
    QList<AbstractPropertyPage*> modelPages = model->makePropertyPages();
    QList<AbstractPropertyPage*>::const_iterator i;
    for (i = modelPages.constBegin(); i != modelPages.constEnd(); i++) {
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
| This just forwards the signal on with the model.                  |
+------------------------------------------------------------------*/
void PropertiesWindow::locateActionTriggered() {
    emit locateWindow(model);
}

/*------------------------------------------------------------------+
| Opens the "Save File" dialog then writes the properties to the    |
| selected file in either text or xml format.                       |
+------------------------------------------------------------------*/
void PropertiesWindow::saveToFile() {
    String fileName = QFileDialog::getSaveFileName(this, tr("Save Window Properties"),
                        QDir::homePath(), "XML Files (*.xml);;All Files (*.*)");
    if (fileName.isEmpty()) {
        return;    // User cancelled
    }

    QFile file(fileName);
    if (!file.open(QFile::WriteOnly)) {
        String msg = tr("Could not open file for writing.")+"\n\""+fileName+"\"";
        QMessageBox::warning(this, tr("Save Window Properties"), msg);
        Logger::error(msg);
        return;
    }
    
    QXmlStreamWriter stream(&file);
    stream.setAutoFormatting(true);
    stream.setAutoFormattingIndent(4);  // 4 spaces
    stream.writeStartDocument();
    stream.writeComment("\n" %
            TR("Properties for window ") % model->getDisplayName() % "\n" %
            TR("Created by Window Detective") % "\n");
    model->toXmlStream(stream);
    stream.writeEndDocument();
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
