/////////////////////////////////////////////////////////////////////
// File: SearchResultsWindow.cpp                                   //
// Date: 22/6/10                                                   //
// Desc: A window to display the results of a search. The results  //
//   are shown in a list with columns similar to that in the       //
//   hierarchy tree. A summary of the search criteria is also      //
//   displayed.                                                    //
/////////////////////////////////////////////////////////////////////

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
          stream << "    Text - \"" << searchCriteria.textToFind << "\"";
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