/////////////////////////////////////////////////////////////////////
// File: WindowTree.h                                              //
// Date: 28/4/10                                                   //
// Desc: Subclass of QTreeWidget to provide a custom widget for    //
//   displaying the window hierarchy.                              //
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

#ifndef WINDOW_TREE_H
#define WINDOW_TREE_H

#include "window_detective/include.h"
#include "TreeItem.h"
#include "ui/ActionManager.h"
#include "inspector/inspector.h"
using namespace inspector;

class MainWindow;  // Forward declaration

class WindowTree : public QTreeWidget {
    Q_OBJECT
private:
    bool columnResizeDisabled; // Optimisation: disable when expanding all items
    QMenu windowMenu;          // Context menu for window items
public:
    WindowTree(QWidget *parent = 0);
    ~WindowTree() {}

    virtual void build();
    virtual void buildHeader();
    void addWindowChildren(WindowItem*);
    WindowItem* findWindowItem(Window* window);
    bool hasItem(TreeItem* item);
    QList<Window*> getSelectedWindows();
    void expandAll();
    void expandSelected();
    void resizeAllColumns();
    void beginExpanding() {columnResizeDisabled=true;}
    void endExpanding() {columnResizeDisabled=false;resizeAllColumns();}
protected slots:
    virtual void insertNewWindow(Window* window);
    virtual void removeWindow(Window* window);
    void treeItemExpanded(QTreeWidgetItem* item);
};

class ProcessWindowTree : public WindowTree {
    Q_OBJECT
private:
    QMenu processMenu;         // Context menu for process items
public:
    ProcessWindowTree(QWidget *parent = 0);
    ~ProcessWindowTree() {}

    void build();
    void buildHeader();
    void addProcessChildren(ProcessItem*, const WindowList&);
    ProcessItem* findProcessItem(Process* process);
    QList<Process*> getSelectedProcesses();
protected slots:
    void insertNewWindow(Window* window);
    void insertNewProcess(Process* process);
    void removeProcess(Process* process);
};

#endif   // WINDOW_TREE_H