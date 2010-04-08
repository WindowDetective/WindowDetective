/////////////////////////////////////////////////////////////////////
// File: TreeItem.cpp                                              //
// Date: 12/3/10                                                   //
// Desc: Represents an item in the window hierarchy tree.          //
//   Two subclasses are defined: ProcessItem for process objects   //
//   and WindowItem for window objects. Each display their own     //
//   info and have their own context menu.                         //
/////////////////////////////////////////////////////////////////////

#include "TreeItem.h"
#include "window_detective/Settings.h"
#include "StringRenderer.h"


/****************/
/*** TreeItem ***/
/****************/

TreeItem::~TreeItem() {
    if (changeTimer)
        delete changeTimer;
}

/*-----------------------------------------------------------------+
 | Sets up common properties for all subclasses.                   |
 +-----------------------------------------------------------------*/
void TreeItem::initialize() {
    changeTimer = NULL;
    setupData();
}

void TreeItem::update() {
    setupData();
    highlightVisible(ItemChanged);
}

/*-----------------------------------------------------------------+
 | Returns this item's ancestor, that is, it's top-level parent.   |
 +-----------------------------------------------------------------*/
TreeItem* TreeItem::ancestor() {
    TreeItem* parent = (TreeItem*)(((QTreeWidgetItem*)this)->parent());
    return parent ? parent->ancestor() : this;
}

/*-----------------------------------------------------------------+
 | Recursively expands all children of this item.                  |
 +-----------------------------------------------------------------*/
void recursivelyExpandItem(QTreeWidgetItem* item, int level) {
    if (level > MAX_EXPAND_LEVEL) return;
    item->setExpanded(true);
    for (int i = 0; i < item->childCount(); i++) {
        recursivelyExpandItem(item->child(i), level+1);
    }
}
void TreeItem::expandAllChildren() {
    recursivelyExpandItem(this, 0);
}

/*-----------------------------------------------------------------+
 | Recursively expands each ancestor of this item.                 |
 | Used to locate this item in the tree.                           |
 +-----------------------------------------------------------------*/
void recursivelyExpandAncestor(QTreeWidgetItem* child) {
    QTreeWidgetItem* parent = child->parent();
    if (parent) {
        parent->setExpanded(true);
        recursivelyExpandAncestor(parent);
    }
}
void TreeItem::expandAncestors() {
    recursivelyExpandAncestor(this);
}

/*-----------------------------------------------------------------+
 | Highlights this item using a style based on the given reason    |
 | and whether it is the immeditae item or an ancestor of it. The  |
 | style is removed after Settings::changedDuration milliseconds   |
 +-----------------------------------------------------------------*/
void TreeItem::highlight(UpdateReason reason, bool isImmediate) {
    // TODO: Other update reasons and take style from Settings
    switch (reason) {
      case ItemChanged: {
          QFont highlightFont = font(0);
          highlightFont.setBold(true);
          setFont(0, highlightFont);
          break;
      }
      case ItemCreated: {
      }
      case ItemDeleted: {
      }
    }

    // Set (or reset) timeout for highlight
    if (!changeTimer) {
        changeTimer = new QTimer();
        changeTimer->setSingleShot(true);
        connect(changeTimer, SIGNAL(timeout()), this, SLOT(unhighlight()));
    }
    changeTimer->start(Settings::treeChangeDuration);
}

/*-----------------------------------------------------------------+
 | Walks up the tree until it reaches the top-level item then      |
 | highlights either this item if it's visible (i.e. all ancestors |
 | are expanded) or the closest visible ancestor of this.          |
 +-----------------------------------------------------------------*/
int recusriveHighlightVisible(TreeItem* baseItem,
                              TreeItem* currentItem,
                              UpdateReason reason) {
    TreeItem* parent = (TreeItem*)(((QTreeWidgetItem*)currentItem)->parent());

    int result = parent ? recusriveHighlightVisible(baseItem,parent,reason) : 1;
    if (result == -1) {
        return -1;      // -1 means we're done, just continue up the stack
    }
    else if (result) {
        if (currentItem == baseItem) {
            currentItem->highlight(reason, true);
            return -1;
        }
        else {
            return (int)currentItem->isExpanded();
        }
    }
    else {
        parent->highlight(reason, false);
        return -1;
    }
}
void TreeItem::highlightVisible(UpdateReason reason) {
    recusriveHighlightVisible(this, this, reason);
}

void TreeItem::unhighlight() {
    setFont(0, treeWidget()->font());
}

/*******************/
/*** ProcessItem ***/
/*******************/

/*-----------------------------------------------------------------+
 | ProcessItem constructors                                        |
 +-----------------------------------------------------------------*/
ProcessItem::ProcessItem() :
    TreeItem(ProcessItemType) {
}
ProcessItem::ProcessItem(Process* process, QTreeWidget* parent) :
    TreeItem(parent, ProcessItemType),
    process(process) {
    initialize();
}
ProcessItem::ProcessItem(Process* process, QTreeWidgetItem* parent) :
    TreeItem(parent, ProcessItemType),
    process(process) {
    initialize();
}

void ProcessItem::setupData() {
    // First column: process name and icon
    setText(0, process->getName());
    setIcon(0, process->getIcon());
    setToolTip(0, tooltipText());

    // Second column: process id
    setText(1, String::number(process->getId()));

    // Everything else is blank as it is not relevant
}

String ProcessItem::tooltipText() {
    return process->getFilePath();
}


/******************/
/*** WindowItem ***/
/******************/

/*-----------------------------------------------------------------+
 | WindowItem constructors                                         |
 +-----------------------------------------------------------------*/
WindowItem::WindowItem() :
    TreeItem(WindowItemType) {
}
WindowItem::WindowItem(Window* window, QTreeWidget* parent) :
    TreeItem(parent, WindowItemType),
    window(window) {
    initialize();
}
WindowItem::WindowItem(Window* window, QTreeWidgetItem* parent) :
    TreeItem(parent, WindowItemType),
    window(window) {
    initialize();
}

void WindowItem::initialize() {
    TreeItem::initialize();
    connect(window, SIGNAL(updated()), this, SLOT(update()));
}

/*-----------------------------------------------------------------+
 | Sets the item's properties from the window model.               |
 +-----------------------------------------------------------------*/
void WindowItem::setupData() {
    // First colums: window class name and icon
    setText(0, window->getWindowClass()->getDisplayName());
    setIcon(0, window->getIcon());
    setToolTip(0, tooltipText());

    if (Settings::greyHiddenWindows && !window->isVisible()) {
        setForeground(0, QBrush(HIDDEN_WINDOW_COLOUR));
        setIcon(0, QIcon(window->getIcon().pixmap(16, QIcon::Disabled)));
    }

    // Second column: window handle
    setText(1, stringLabel(window->getHandle()));

    // Third column: window title/text
    String text = window->getText();
    if (text.length() > 200) {
        text = text.left(200) + "...";
    }
    setText(2, stringLabel(text.simplified()));

    // Forth column: window dimensions
    setText(3, stringLabel(window->getDimensions()));
}

/*-----------------------------------------------------------------+
 | Constructs a HTML string for use as the item's tooltip.         |
 +-----------------------------------------------------------------*/
String WindowItem::tooltipText() {
    String tooltipString;
    QTextStream stream(&tooltipString);

    stream << "<html><table><tr><td><b>Class:</b></td><td>"
           << stringLabel(window->getWindowClass())
           << "<td><tr><td><b>Handle:</b></td><td>"
           << stringLabel(window->getHandle()) << "</td>";
    if (!window->getText().isEmpty()) {
        String text = window->getText();
        if (text.length() > 200) {
            text = text.left(200) + "...";
        }
        stream << "<tr><td><b>Text:</b></td><td>"
               <<stringLabel(text.simplified()) << "<td>";
    }
    stream << "</tr></table>";
    if (!window->isVisible()) {
        stream << "<br/>Window is not visible";
    }
    if (window->getChildren().size() > 0) {
        stream << "<br/>Has " << String::number(window->getChildren().size())
               << " immediate children";
    }
    stream << "<br/>Window is " << String(window->isUnicode() ? "Unicode" : "ANSI")
           << "</html>";

    return tooltipString;
}
