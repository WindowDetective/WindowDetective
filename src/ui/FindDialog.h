/////////////////////////////////////////////////////////////////////
// File: FindDialog.h                                              //
// Date: 15/3/10                                                   //
// Desc: The find window. Allows users to search for a window by a //
//   number of different criteria, including name, handle, style.  //
//   If only one window is found, it is selected in the window     //
//   tree. If multiple windows are found, they are displayed in a  //
//   separate list window.                                         //
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

#ifndef FIND_DIALOG_H
#define FIND_DIALOG_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "inspector/SearchCriteria.h"
#include "forms/ui_FindDialog.h"
using namespace inspector;

class MainWindow;  // Forward declaration

class FindDialog : public QDialog, private Ui::FindDialog {
    Q_OBJECT
private:
    MainWindow* mainWindow;
public:
    FindDialog(MainWindow* mainWindow, QWidget* parent = 0);
    ~FindDialog() {}

signals:
    void singleWindowFound(Window*);
protected:
    void readSmartSettings();
    void writeSmartSettings();
    void showEvent(QShowEvent* e);
    void hideEvent(QHideEvent* e);
    void openResultsWindow(WindowList, SearchCriteria);
private slots:
    void windowTextSelected();
    void handleSelected();
    void windowClassSelected();
    void findButtonClicked();
};

#endif   // FIND_DIALOG_H