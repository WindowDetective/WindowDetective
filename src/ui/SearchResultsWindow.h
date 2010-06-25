/////////////////////////////////////////////////////////////////////
// File: SearchResultsWindow.h                                     //
// Date: 22/6/10                                                   //
// Desc: A window to display the results of a search. The results  //
//   are shown in a list with columns similar to that in the       //
//   hierarchy tree. A summary of the search criteria is also      //
//   displayed.                                                    //
/////////////////////////////////////////////////////////////////////

#ifndef SEARCH_RESULTS_WINDOW_H
#define SEARCH_RESULTS_WINDOW_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "inspector/SearchCriteria.h"
#include "ui/custom_widgets/TreeItem.h"
#include "forms/ui_SearchResultsWindow.h"
using namespace inspector;

class SearchResultsWindow : public QDialog, private Ui::SearchResultsWindow {
    Q_OBJECT
private:
    SearchCriteria searchCriteria;
public:
    SearchResultsWindow(QWidget* parent = 0);
    ~SearchResultsWindow() {}

    void openOn(WindowList windows, SearchCriteria searchCriteria);
    void populateResultsList(WindowList& windows);
private slots:
    void searchAgain();
};

#endif   // SEARCH_RESULTS_WINDOW_H