/////////////////////////////////////////////////////////////////////
// File: SearchResultsWindow.cpp                                   //
// Date: 22/6/10                                                   //
// Desc: A window to display the results of a search. The results  //
//   are shown in a list with columns similar to that in the       //
//   hierarchy tree. A summary of the search criteria is also      //
//   displayed.                                                    //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010 XTAL256

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

#include "SearchResultsWindow.h"
#include "inspector/inspector.h"
#include "inspector/WindowManager.h"
#include "inspector/SearchCriteria.h"
using namespace inspector;

SearchResultsWindow::SearchResultsWindow(QWidget* parent) :
    QDialog(parent) {
    setupUi(this);

    // TODO: Make some sort of "column manager" which allows users to customise
    //  the columns here and in the trees (either separately or as one).
    QStringList columnLabels;
    columnLabels << "Window" << "Handle" << "Text" << "Dimensions";
    windowList->setColumnCount(4);
    windowList->setHeaderLabels(columnLabels);

    connect(repeatButton, SIGNAL(clicked()), this, SLOT(searchAgain()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
}

void SearchResultsWindow::openOn(WindowList windows,
                                 SearchCriteria searchCriteria) {
    this->searchCriteria = searchCriteria;
    populateResultsList(windows);
    show();
}

void SearchResultsWindow::populateResultsList(WindowList& windows) {
    // "Abuse" the QTreeWidget by only using top-level items to make it
    // look like a list view with columns.
    windowList->clear();

    WindowList::const_iterator i;
    for (i = windows.begin(); i != windows.end(); i++) {
        // Note: Tree widget takes ownership of new item (see Qt docs)
        new WindowItem(*i, windowList);
    }
    windowList->resizeAllColumns();

    setWindowTitle(String(APP_NAME) + " - Found " +
                String::number(windows.size()) + " windows");
    String criteriaString;
    QTextStream stream(&criteriaString);
    stream << "Search Criteria:\n";
    /*** TODO *********************************
     This is just for basic search. It should be removed once
     i get SearchCriteriaItem working
    */
    switch (searchCriteria.type) {
      case 0: {
          stream << (searchCriteria.useRegex ? "    Regex - \"" : "    Text - \"")
                 << searchCriteria.textToFind << "\"";
          break;
      }
      case 1: {
          stream << "    Handle - " << hexString(searchCriteria.handleToFind);
          break;
      }
      case 2: {
          stream << "    Class - " << searchCriteria.classToFind;
          break;
      }
    }
    /******************************************/
    criteriaLabel->setText(criteriaString);
}

/**********************/
/*** Event handlers ***/
/**********************/

/*------------------------------------------------------------------+
| Runs the search again and re-populates the list with the results. |
+------------------------------------------------------------------*/
void SearchResultsWindow::searchAgain() {
    WindowList foundWindows = WindowManager::current()->find(searchCriteria);
    populateResultsList(foundWindows);
}