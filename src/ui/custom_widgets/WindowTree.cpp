/////////////////////////////////////////////////////////////////////
// File: WindowTree.cpp                                            //
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


#include "WindowTree.h"
#include "ui/MainWindow.h"
#include "inspector/WindowManager.h"


/******************/
/*** WindowTree ***/
/******************/

WindowTree::WindowTree(QWidget *parent) :
    QTreeWidget(parent),
    columnResizeDisabled(false),
    windowMenu() {
    setSelectionMode(QAbstractItemView::ExtendedSelection);
    connect(WindowManager::current(), SIGNAL(windowAdded(Window*)), this, SLOT(insertNewWindow(Window*)));
    connect(WindowManager::current(), SIGNAL(windowRemoved(Window*)), this, SLOT(removeWindow(Window*)));
    connect(this, SIGNAL(itemExpanded(QTreeWidgetItem*)), this, SLOT(treeItemExpanded(QTreeWidgetItem*)));
}

void WindowTree::buildHeader() {
    QStringList desktopTreeLabels;
    desktopTreeLabels << "Window" << "Handle" << "Text" << "Dimensions";
    setColumnCount(4);
    setHeaderLabels(desktopTreeLabels);
}

/*------------------------------------------------------------------+
| Builds/rebuilds the tree. Note: window data should be refreshed   |
| before this methods is called.                                    |
+------------------------------------------------------------------*/
void WindowTree::build() {
    WindowManager* manager = WindowManager::current();

    clear();
    WindowItem* top = new WindowItem(manager->getDesktopWindow(), this);
    top->setExpanded(true);
    addWindowChildren(top);
    resizeAllColumns();
}

/*------------------------------------------------------------------+
| Recursively adds window children to the tree.                     |
+------------------------------------------------------------------*/
void WindowTree::addWindowChildren(WindowItem* item) {
    WindowList children = item->getWindow()->getChildren();

    WindowList::const_iterator i;
    for (i = children.begin(); i != children.end(); i++) {
        // Note: Parent item takes ownership of new item (see Qt docs)
        addWindowChildren(new WindowItem(*i, item));
    }
}

/*------------------------------------------------------------------+
| Recursively searches all tree items to find one with the given    |
| window. Returns NULL if it can't find one.                        |
+------------------------------------------------------------------*/
WindowItem* findWindowItemRecursive(QTreeWidgetItem* item, Window* window) {
    WindowItem* windowItem = dynamic_cast<WindowItem*>(item);

    // Check if this item is the one we're looking for
    if (windowItem && windowItem->getWindow() == window)
        return windowItem;

    // Now recursively check all children
    WindowItem* result = NULL;
    for (int i = 0; i < item->childCount(); i++) {
        result = findWindowItemRecursive(item->child(i), window);
        if (result) return result;
    }
    return NULL;
}
WindowItem* WindowTree::findWindowItem(Window* window) {
    if (!window)
        return NULL;
    QTreeWidgetItem* root = invisibleRootItem();
    return findWindowItemRecursive(root, window);
}

/*------------------------------------------------------------------+
| Recursively searches all tree items to find the one given.        |
| Returns true if it is found, false if not.                        |
+------------------------------------------------------------------*/
bool hasItemRecursive(QTreeWidgetItem* currentItem, TreeItem* itemToFind) {
    // Check if this item is the one we're looking for
    if (currentItem == itemToFind)
        return true;

    // Now recursively check all children
    bool result = false;
    for (int i = 0; i < currentItem->childCount(); i++) {
        result = hasItemRecursive(currentItem->child(i), itemToFind);
        if (result) return result;
    }
    return false;
}

bool WindowTree::hasItem(TreeItem* item) {
    QTreeWidgetItem* root = invisibleRootItem();
    return hasItemRecursive(root, item);
}

/*------------------------------------------------------------------+
| Returns the window objects of the currently selected items. If no |
| item is selected, or it is not a window item, NULL is returned.   |
+------------------------------------------------------------------*/
QList<Window*> WindowTree::getSelectedWindows() {
    QList<Window*> windows;
    WindowItem* windowItem = NULL;
    Window* window = NULL;

    foreach (QTreeWidgetItem* item, selectedItems()) {
        windowItem = dynamic_cast<WindowItem*>(item);
        if (windowItem) {
            window = windowItem->getWindow();
            if (window) windows.append(window);
        }
    }
    return windows;
}

void WindowTree::insertNewWindow(Window* window) {
    WindowItem* parentItem = findWindowItem(window->getParent());
    if (!parentItem) return;

    // Note: Tree takes ownership of new item (see Qt docs)
    WindowItem* item = new WindowItem(window, parentItem);
    item->update(WindowCreated);
}

void WindowTree::removeWindow(Window* window) {
    WindowItem* item = findWindowItem(window);
    if (!item) return;

    item->update(WindowDestroyed);
}

/*------------------------------------------------------------------+
| Expand all items and their children.                              |
+------------------------------------------------------------------*/
void WindowTree::expandAll() {
    TreeItem* item = dynamic_cast<TreeItem*>(invisibleRootItem());
    if (item) item->expandAllChildren();
}

/*------------------------------------------------------------------+
| Expand the selected items and their children.                     |
+------------------------------------------------------------------*/
void WindowTree::expandSelected() {
    foreach (QTreeWidgetItem* qItem, selectedItems()) {
      TreeItem* item = dynamic_cast<TreeItem*>(qItem);
      item->expandAllChildren();
    }
}

/*------------------------------------------------------------------+
| Resize all columns to fit their contents. Temporarily limit the   |
| maximum width of each column to 300 so that we don't end up with  |
| extremely wide columns (user is free to make them bigger though). |
+------------------------------------------------------------------*/
void WindowTree::resizeAllColumns() {
    for (int i = 0; i < columnCount(); i++) {
        resizeColumnToContents(i);
        if (columnWidth(i) > 300) {
            setColumnWidth(i, 300);
        }
    }
}

/*------------------------------------------------------------------+
| When an item is expanded, resize the first column to fit it's     |
| children in view.                                                 |
+------------------------------------------------------------------*/
void WindowTree::treeItemExpanded(QTreeWidgetItem*) {
    // If all items are expanding, we don't want to resize *every* time
    if (!columnResizeDisabled)
        resizeAllColumns();
}


/*************************/
/*** ProcessWindowTree ***/
/*************************/

ProcessWindowTree::ProcessWindowTree(QWidget *parent) :
    WindowTree(parent),
    processMenu() {
    connect(WindowManager::current(), SIGNAL(processAdded(Process*)), this, SLOT(insertNewProcess(Process*)));
    connect(WindowManager::current(), SIGNAL(processRemoved(Process*)), this, SLOT(removeProcess(Process*)));
}

void ProcessWindowTree::buildHeader() {
    QStringList processTreeLabels;
    processTreeLabels << "Window/Process" << "Handle" << "Text" << "Dimensions";
    setColumnCount(4);
    setHeaderLabels(processTreeLabels);
}

/*------------------------------------------------------------------+
| Builds/rebuilds the tree using the window and process objects.    |
| Note: window data should be refreshed before this methods is      |
| called.                                                           |
+------------------------------------------------------------------*/
void ProcessWindowTree::build() {
    WindowManager* manager = WindowManager::current();
    ProcessItem* processItem;

    clear();
    WindowList topWindows = manager->getDesktopWindow()->getChildren();
    for (int i = 0; i < manager->allProcesses.size(); i++) {
        processItem = new ProcessItem(manager->allProcesses[i], this);
        addProcessChildren(processItem, topWindows);
    }
    resizeAllColumns();
}

/*------------------------------------------------------------------+
| Recursively adds a process's top-level windows and their          |
| children to the tree.                                             |
+------------------------------------------------------------------*/
void ProcessWindowTree::addProcessChildren(ProcessItem* item,
                        const WindowList& allTopWindows) {
    WindowList topWindows;
    WindowList::const_iterator i;

    // Find all top-level windows owned by the process
    for (i = allTopWindows.begin(); i != allTopWindows.end(); i++) {
        if ((*i)->getProcess() == item->getProcess()) {
            // Note: Parent item takes ownership of new item (see Qt docs)
            addWindowChildren(new WindowItem(*i, item));
        }
    }
}

/*------------------------------------------------------------------+
| Recursively searches all tree items to find one with the given    |
| process. Returns NULL if it can't find one.                       |
+------------------------------------------------------------------*/
ProcessItem* findProcessItemRecursive(QTreeWidgetItem* item, Process* process) {
    ProcessItem* processItem = dynamic_cast<ProcessItem*>(item);

    // Check if this item is the one we're looking for
    if (processItem && processItem->getProcess() == process)
        return processItem;

    // Now recursively check all children
    ProcessItem* result = NULL;
    for (int i = 0; i < item->childCount(); i++) {
        result = findProcessItemRecursive(item->child(i), process);
        if (result) return result;
    }
    return NULL;
}
ProcessItem* ProcessWindowTree::findProcessItem(Process* process) {
    if (!process)
        return NULL;
    QTreeWidgetItem* root = invisibleRootItem();
    return findProcessItemRecursive(root, process);
}

/*------------------------------------------------------------------+
| Returns the process objects of the currently selected items. If no|
| item is selected, or it is not a process item, NULL is returned.  |
+------------------------------------------------------------------*/
QList<Process*> ProcessWindowTree::getSelectedProcesses() {
    QList<Process*> processes;
    ProcessItem* processItem = NULL;
    Process* process = NULL;

    foreach (QTreeWidgetItem* item, selectedItems()) {
        processItem = dynamic_cast<ProcessItem*>(item);
        if (processItem) {
            process = processItem->getProcess();
            if (process) processes.append(process);
        }
    }
    return processes;
}

void ProcessWindowTree::insertNewWindow(Window* window) {
    WindowManager* manager = WindowManager::current();
    TreeItem* parentItem = NULL;

    // Check if it's a top-level window
    if (window->getParent() == manager->getDesktopWindow()) {
        parentItem = findProcessItem(window->getProcess());
    }
    else {
        parentItem = findWindowItem(window->getParent());
    }
    if (!parentItem) return;

    // Note: parentItem takes ownership of new item (see Qt docs)
    WindowItem* item = new WindowItem(window, parentItem);
    item->update(WindowCreated);
}

void ProcessWindowTree::insertNewProcess(Process* process) {
    // Note: This tree takes ownership of new item (see Qt docs)
    new ProcessItem(process, this);
}

void ProcessWindowTree::removeProcess(Process* process) {
    ProcessItem* item = findProcessItem(process);
    if (!item) return;

    item->update(WindowDestroyed);
}
