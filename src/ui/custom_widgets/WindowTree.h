/////////////////////////////////////////////////////////////////////
// File: WindowTree.h                                              //
// Date: 28/4/10                                                   //
// Desc: Subclass of QTreeWidget to provide a custom widget for    //
//   displaying the window hierarchy.                              //
/////////////////////////////////////////////////////////////////////

#ifndef WINDOW_TREE_H
#define WINDOW_TREE_H

#include "window_detective/include.h"
#include "TreeItem.h"
#include "inspector/inspector.h"
using namespace inspector;

class WindowTree : public QTreeWidget {
    Q_OBJECT
public:
    WindowTree(QWidget *parent = 0);
    ~WindowTree() {}

    virtual void build();
    virtual void buildHeader();
    void addWindowChildren(WindowItem*);
    WindowItem* findWindowItem(Window* window);
protected slots:
    virtual void insertNewWindow(Window* window);
    virtual void removeWindow(Window* window);
};

class ProcessWindowTree : public WindowTree {
    Q_OBJECT
public:
    ProcessWindowTree(QWidget *parent = 0);
    ~ProcessWindowTree() {}

    void build();
    void buildHeader();
    void addProcessChildren(ProcessItem*, const QList<Window*>&);
    ProcessItem* findProcessItem(Process* process);
protected slots:
    void insertNewWindow(Window* window);
    void insertNewProcess(Process* process);
    void removeProcess(Process* process);
};

#endif   // WINDOW_TREE_H