/////////////////////////////////////////////////////////////////////
// File: MainWindow.h                                              //
// Date: 15/2/10                                                   //
// Desc: The main UI window which is shown when the app starts.    //
/////////////////////////////////////////////////////////////////////

#include "MainWindow.h"
#include "inspector/WindowManager.h"
#include "window_detective/Settings.h"
#include "TreeItem.h"
using namespace inspector;

MainWindow::MainWindow(QMainWindow *parent) :
    QMainWindow(parent),
    flashHighlighter(false),
    preferencesWindow(),
    selectedWindow(NULL) {
    setupUi(this);

    flashHighlighter.create();
    picker = new WindowPicker(pickerToolBar, this);
    pickerToolBar->addWidget(picker);

    // Since there doesn't seem to be any way to control the size of dock
    // widgets, we have to force a max/min size. These size limits are
    // removed when the window is shows, after the layout manager has sized them
    treeDock->setMinimumWidth(300);
    statusDock->setMaximumHeight(180);

    // UI events
    connect(actionPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
    connect(actionFind, SIGNAL(triggered()), this, SLOT(openFindDialog()));
    connect(actionRefresh, SIGNAL(triggered()), this, SLOT(refreshWindowTree()));
    connect(desktopWindowTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTreeContextMenu(const QPoint&)));
    connect(processWindowTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTreeContextMenu(const QPoint&)));
    connect(treeTabs, SIGNAL(currentChanged(int)), this, SLOT(treeTabChanged(int)));
    connect(desktopWindowTree, SIGNAL(itemSelectionChanged()), this, SLOT(selectedWindowChanged()));
    connect(processWindowTree, SIGNAL(itemSelectionChanged()), this, SLOT(selectedWindowChanged()));

    // Window menu actions
    connect(actionExpandAll, SIGNAL(triggered()), this, SLOT(expandTreeItem()));
    connect(actionViewWindowProperties, SIGNAL(triggered()), this, SLOT(viewWindowProperties()));
    connect(actionSetWindowProperties, SIGNAL(triggered()), this, SLOT(setWindowProperties()));
    connect(actionSetStyles, SIGNAL(triggered()), this, SLOT(setWindowStyles()));
    connect(actionShowWindow, SIGNAL(triggered()), this, SLOT(showWindow()));
    connect(actionHideWindow, SIGNAL(triggered()), this, SLOT(hideWindow()));
    connect(actionFlashWindow, SIGNAL(triggered()), this, SLOT(flashWindow()));
    connect(actionCloseWindow, SIGNAL(triggered()), this, SLOT(closeWindow()));

    // Other events
    connect(&preferencesWindow, SIGNAL(highlightWindowChanged()), &picker->highlighter, SLOT(update()));
    connect(&findDialog, SIGNAL(windowsFound(QList<Window*>)), this, SLOT(windowsFound(QList<Window*>)));
    connect(picker, SIGNAL(windowPicked(Window*)), this, SLOT(locateWindowInTree(Window*)));

    // TODO: Set this when the soft values are loaded
    currentTree = (treeTabs->currentIndex() == 0 ? desktopWindowTree : processWindowTree);

    buildTreeHeaders();
    refreshWindowTree();
}

MainWindow::~MainWindow() {
    delete picker;
}

/*-----------------------------------------------------------------+
 | Adds column headers to the window trees. The first column, the  |
 | actual tree, cannot be changed. All others can be added, moved  |
 | or removed by the user.                                         |
 +-----------------------------------------------------------------*/
void MainWindow::buildTreeHeaders() {
    QStringList desktopTreeLabels, processTreeLabels;

    desktopTreeLabels << "Window" << "Handle" << "Text" << "Dimensions";
    desktopWindowTree->setColumnCount(4);
    desktopWindowTree->setHeaderLabels(desktopTreeLabels);

    processTreeLabels << "Window/Process" << "Handle" << "Text" << "Dimensions";
    processWindowTree->setColumnCount(4);
    processWindowTree->setHeaderLabels(processTreeLabels);
}

/*-----------------------------------------------------------------+
 | Helper function to recursively add window children to the tree. |
 +-----------------------------------------------------------------*/
void addWindowChildren(QTreeWidget* tree, WindowItem* item) {
    QList<Window*> children = item->getWindow()->getChildren();

    QList<Window*>::const_iterator i;
    for (i = children.begin(); i != children.end(); i++) {
        addWindowChildren(tree, new WindowItem(*i, item));
    }
}

/*-----------------------------------------------------------------+
 | Helper function to recursively add a process's top-level        |
 | windows and their children to the tree.                         |
 +-----------------------------------------------------------------*/
void addProcessChildren(QTreeWidget* tree, ProcessItem* item,
                        const QList<Window*>& allTopWindows) {
    QList<Window*> topWindows;
    QList<Window*>::const_iterator i;

    // Find all top-level windows owned by the process
    for (i = allTopWindows.begin(); i != allTopWindows.end(); i++) {
        if ((*i)->getProcess() == item->getProcess()) {
            addWindowChildren(tree, new WindowItem(*i, item));
        }
    }
}

/*-----------------------------------------------------------------+
 | Builds/rebuilds the window hierarchy trees using the window and |
 | process objects. Note: window data should be refreshed before   |
 | this methods is called.                                         |
 +-----------------------------------------------------------------*/
void MainWindow::builtTrees() {
    WindowManager* manager = WindowManager::getCurrent();

    desktopWindowTree->clear();
    WindowItem* top = new WindowItem(manager->getDesktopWindow(), desktopWindowTree);
    top->setExpanded(true);
    addWindowChildren(desktopWindowTree, top);
    desktopWindowTree->setColumnWidth(0, 200);

    processWindowTree->clear();
    ProcessItem* processItem;
    QList<Window*> topWindows = manager->getDesktopWindow()->getChildren();
    for (int i = 0; i < manager->allProcesses.size(); i++) {
        // Only show processes that actually have windows
        if (manager->allProcesses[i]->hasWindows()) {
            processItem = new ProcessItem(manager->allProcesses[i], processWindowTree);
            addProcessChildren(processWindowTree, processItem, topWindows);
        }
    }
    processWindowTree->setColumnWidth(0, 200);
}

/*-----------------------------------------------------------------+
 | Expands the items in the current tree to expose and highlight   |
 | the item corresponding to the given window.                     |
 +-----------------------------------------------------------------*/
void MainWindow::locateWindowInTree(Window* window) {
    WindowItem* item = NULL;
    // Match item by handle string, which should be unique
    // TODO: Might be faster to write my own recursive algorithm to
    // find by window object (pointer) rather than string
    QList<QTreeWidgetItem*> found = currentTree->findItems(
                           hexString((uint)window->getHandle()),
                           Qt::MatchFixedString | Qt::MatchCaseSensitive |
                           Qt::MatchWrap | Qt::MatchRecursive, 1);
    if (!found.isEmpty()) {
        item = dynamic_cast<WindowItem*>(found.first());
        if (item) {
            item->expandAncestors();
            currentTree->setCurrentItem(item);
            if (isShiftDown()) viewWindowProperties();
            // if (ctrl) view messages
        }
    }
}

/*-----------------------------------------------------------------+
 | Creates and displays a list widget containing the given windows.|
 +-----------------------------------------------------------------*/
void MainWindow::openWindowList(QList<Window*> windows) {
    // TODO: Provide more functionality in this widget, including
    //  context menu and columns
    QListWidget* listWidget = new QListWidget();
    QListWidgetItem* item = NULL;

    foreach (Window* window, windows) {
        String text = window->getWindowClass()->getName();
        if (!window->getText().isEmpty())
            text += " - " + window->getText();
        item = new QListWidgetItem(window->getIcon(), text);
        listWidget->addItem(item);
    }

    String title = String(APP_NAME) + " - Found " +
                String::number(windows.size()) + " windows";
    listWidget->setWindowTitle(title);
    listWidget->setAttribute(Qt::WA_DeleteOnClose);
    listWidget->resize(350, 400);
    listWidget->show();
}


/**********************/
/*** Event handlers ***/
/**********************/

void MainWindow::refreshWindowTree() {
    // Rebuilding the tree will destroy all Window objects.
    // That will invalidate any property windows, so we disable them.
    QList<QMdiSubWindow*> mdiWindows = mdiArea->subWindowList();
    foreach (QMdiSubWindow* each, mdiWindows) {
        each->widget()->setEnabled(false);
    }

    WindowManager::getCurrent()->refreshAllWindows();
    builtTrees();
}

void MainWindow::openPreferences() {
    if (preferencesWindow.isVisible()) {
        preferencesWindow.activateWindow();
        preferencesWindow.raise();
    }
    else {
        preferencesWindow.show();
    }
}

void MainWindow::openFindDialog() {
    if (findDialog.isVisible()) {
        findDialog.activateWindow();
        findDialog.raise();
    }
    else {
        findDialog.show();
    }
}

void MainWindow::treeTabChanged(int tabIndex) {
    currentTree = (tabIndex == 0 ? desktopWindowTree : processWindowTree);
}

void MainWindow::selectedWindowChanged() {
    selectedWindow = NULL;
    if (currentTree->selectedItems().size() > 0) {
        QTreeWidgetItem* item = currentTree->selectedItems().first();
        WindowItem* windowItem;
        if (item->type() == WindowItemType) {
            if (windowItem = dynamic_cast<WindowItem*>(item)) {
                selectedWindow = windowItem->getWindow();
            }
        }
    }
}

void MainWindow::showTreeContextMenu(const QPoint&) {
    if (currentTree->selectedItems().size() > 0) {
        QTreeWidgetItem* item = currentTree->selectedItems().first();
        if (item->type() == WindowItemType)
            menuWindow->exec(QCursor::pos());
        else if (item->type() == ProcessItemType)
            menuProcess->exec(QCursor::pos());
    }
}

void MainWindow::expandTreeItem() {
    foreach (QTreeWidgetItem* qItem, currentTree->selectedItems()) {
        TreeItem* item = dynamic_cast<TreeItem*>(qItem);
        item->expandAllChildren();
    }
}

void MainWindow::viewWindowProperties() {
    PropertiesWindow* propertiesWindow = new PropertiesWindow(selectedWindow);
    propertiesWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(propertiesWindow, SIGNAL(locateWindow(Window*)), this, SLOT(locateWindowInTree(Window*)));
    connect(selectedWindow, SIGNAL(updated()), propertiesWindow, SLOT(update()));

    // Add the window to the MDI area
    QMdiSubWindow* subWindow = mdiArea->addSubWindow(propertiesWindow);

    // The initial position and size of the window is too small, we will
    // make it 80% of the MDI area's smallest dimension and position it
    // to ensure that it fits in view
    int x, y, size;
    int minDim = qMin(mdiArea->size().width(), mdiArea->size().height());
    size = qMin((int)(minDim * 0.80f), 500);
    x = rand(mdiArea->size().width() - size);
    y = rand(mdiArea->size().height() - size);
    subWindow->setGeometry(x, y, size, size);

    propertiesWindow->show();
}

void MainWindow::setWindowProperties() {
    SetPropertiesDialog* dialog = new SetPropertiesDialog(selectedWindow, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->showAtTab(0);
}

void MainWindow::setWindowStyles() {
    SetPropertiesDialog* dialog = new SetPropertiesDialog(selectedWindow, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->showAtTab(1);
}

void MainWindow::windowsFound(QList<Window*> windows) {
    if (windows.size() == 1) {
        locateWindowInTree(windows.first());
    }
    else {
        openWindowList(windows);
    }
}

void MainWindow::showWindow() {
    if (selectedWindow) selectedWindow->show();
}

void MainWindow::hideWindow() {
    if (selectedWindow) selectedWindow->hide();
}

void MainWindow::flashWindow() {
    if (selectedWindow) flashHighlighter.flash(selectedWindow);
}

void MainWindow::closeWindow() {
    if (selectedWindow) selectedWindow->close();
}

void MainWindow::showEvent(QShowEvent*) {
    // Restore the size limitations that were set in constructor
    treeDock->setMinimumWidth(0);
    statusDock->setMaximumHeight(1000);
}

void MainWindow::closeEvent(QCloseEvent*) {
    Settings::write();
    QApplication::quit();
}