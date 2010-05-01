/////////////////////////////////////////////////////////////////////
// File: WindowTree.cpp                                            //
// Date: 28/4/10                                                   //
// Desc: Subclass of QTreeWidget to provide a custom widget for    //
//   displaying the window hierarchy.                              //
/////////////////////////////////////////////////////////////////////


#include "WindowTree.h"
#include "inspector/WindowManager.h"


/******************/
/*** WindowTree ***/
/******************/

WindowTree::WindowTree(QWidget *parent) :
    QTreeWidget(parent) {
    connect(WindowManager::current(), SIGNAL(windowAdded(Window*)), this, SLOT(insertNewWindow(Window*)));
    connect(WindowManager::current(), SIGNAL(windowRemoved(Window*)), this, SLOT(removeWindow(Window*)));
}

void WindowTree::buildHeader() {
    QStringList desktopTreeLabels;
    desktopTreeLabels << "Window" << "Handle" << "Text" << "Dimensions";
    setColumnCount(4);
    setHeaderLabels(desktopTreeLabels);
}

/*-----------------------------------------------------------------+
 | Builds/rebuilds the tree. Note: window data should be refreshed |
 | before this methods is called.                                  |
 +-----------------------------------------------------------------*/
void WindowTree::build() {
    WindowManager* manager = WindowManager::current();

    clear();
    WindowItem* top = new WindowItem(manager->getDesktopWindow(), this);
    top->setExpanded(true);
    addWindowChildren(top);
    setColumnWidth(0, 200);
}

/*-----------------------------------------------------------------+
 | Recursively adds window children to the tree.                   |
 +-----------------------------------------------------------------*/
void WindowTree::addWindowChildren(WindowItem* item) {
    QList<Window*> children = item->getWindow()->getChildren();

    QList<Window*>::const_iterator i;
    for (i = children.begin(); i != children.end(); i++) {
        // Note: Parent item takes ownership of new item (see Qt docs)
        addWindowChildren(new WindowItem(*i, item));
    }
}

// Match item by handle string, which should be unique
// TODO: Might be faster to write my own recursive algorithm to
// find by window object (pointer) rather than string.
WindowItem* WindowTree::findWindowItem(Window* window) {
    if (!window)
        return NULL;
    QList<QTreeWidgetItem*> found = findItems(
                        hexString((uint)window->getHandle()),
                        Qt::MatchFixedString | Qt::MatchCaseSensitive |
                        Qt::MatchWrap | Qt::MatchRecursive, 1);
    if (!found.isEmpty())
        return dynamic_cast<WindowItem*>(found.first());
    else
        return NULL;
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


/*************************/
/*** ProcessWindowTree ***/
/*************************/

ProcessWindowTree::ProcessWindowTree(QWidget *parent) :
    WindowTree(parent) {
    connect(WindowManager::current(), SIGNAL(processAdded(Process*)), this, SLOT(insertNewProcess(Process*)));
    connect(WindowManager::current(), SIGNAL(processRemoved(Process*)), this, SLOT(removeProcess(Process*)));
}

void ProcessWindowTree::buildHeader() {
    QStringList processTreeLabels;
    processTreeLabels << "Window/Process" << "Handle" << "Text" << "Dimensions";
    setColumnCount(4);
    setHeaderLabels(processTreeLabels);
}

/*-----------------------------------------------------------------+
 | Builds/rebuilds the tree using the window and process objects.  |
 | Note: window data should be refreshed before this methods is    |
 | called.                                                         |
 +-----------------------------------------------------------------*/
void ProcessWindowTree::build() {
    WindowManager* manager = WindowManager::current();
    ProcessItem* processItem;

    clear();
    QList<Window*> topWindows = manager->getDesktopWindow()->getChildren();
    for (int i = 0; i < manager->allProcesses.size(); i++) {
        processItem = new ProcessItem(manager->allProcesses[i], this);
        addProcessChildren(processItem, topWindows);
    }
    setColumnWidth(0, 200);
}

/*-----------------------------------------------------------------+
 | Recursively adds a process's top-level windows and their        |
 | children to the tree.                                           |
 +-----------------------------------------------------------------*/
void ProcessWindowTree::addProcessChildren(ProcessItem* item,
                        const QList<Window*>& allTopWindows) {
    QList<Window*> topWindows;
    QList<Window*>::const_iterator i;

    // Find all top-level windows owned by the process
    for (i = allTopWindows.begin(); i != allTopWindows.end(); i++) {
        if ((*i)->getProcess() == item->getProcess()) {
            // Note: Parent item takes ownership of new item (see Qt docs)
            addWindowChildren(new WindowItem(*i, item));
        }
    }
}

// Match item by handle string, which should be unique
// TODO: Might be faster to write my own recursive algorithm to
// find by process object (pointer) rather than string.
ProcessItem* ProcessWindowTree::findProcessItem(Process* process) {
    if (!process)
        return NULL;
    QList<QTreeWidgetItem*> found = findItems(
                        String::number(process->getId()),
                        Qt::MatchFixedString | Qt::MatchCaseSensitive |
                        Qt::MatchWrap | Qt::MatchRecursive, 1);
    if (!found.isEmpty())
        return dynamic_cast<ProcessItem*>(found.first());
    else
        return NULL;
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