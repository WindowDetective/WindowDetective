/////////////////////////////////////////////////////////////////////
// File: MainWindow.h                                              //
// Date: 15/2/10                                                   //
// Desc: The main UI window which is shown when the app starts.    //
/////////////////////////////////////////////////////////////////////

#include "MainWindow.h"
#include "AboutDialog.h"
#include "inspector/WindowManager.h"
#include "inspector/MessageHandler.h"
#include "window_detective/Settings.h"
#include "custom_widgets/TreeItem.h"
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
    connect(actnPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
    connect(actnFind, SIGNAL(triggered()), this, SLOT(openFindDialog()));
    connect(actnRefresh, SIGNAL(triggered()), this, SLOT(refreshWindowTree()));
    connect(desktopWindowTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTreeContextMenu(const QPoint&)));
    connect(processWindowTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTreeContextMenu(const QPoint&)));
    connect(treeTabs, SIGNAL(currentChanged(int)), this, SLOT(treeTabChanged(int)));
    connect(desktopWindowTree, SIGNAL(itemSelectionChanged()), this, SLOT(selectedWindowChanged()));
    connect(processWindowTree, SIGNAL(itemSelectionChanged()), this, SLOT(selectedWindowChanged()));

    // Window menu actions
    connect(actnExpandAll, SIGNAL(triggered()), this, SLOT(expandTreeItem()));
    connect(actnViewWindowProperties, SIGNAL(triggered()), this, SLOT(viewWindowProperties()));
    connect(actnSetWindowProperties, SIGNAL(triggered()), this, SLOT(setWindowProperties()));
    connect(actnViewWindowMessages, SIGNAL(triggered()), this, SLOT(viewWindowMessages()));
    connect(actnSetStyles, SIGNAL(triggered()), this, SLOT(setWindowStyles()));
    connect(actnShowWindow, SIGNAL(triggered()), this, SLOT(actionShowWindow()));
    connect(actnHideWindow, SIGNAL(triggered()), this, SLOT(actionHideWindow()));
    connect(actnFlashWindow, SIGNAL(triggered()), this, SLOT(actionFlashWindow()));
    connect(actnCloseWindow, SIGNAL(triggered()), this, SLOT(actionCloseWindow()));
    connect(actnAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));

    // Other events
    connect(&preferencesWindow, SIGNAL(highlightWindowChanged()), &picker->highlighter, SLOT(update()));
    connect(&preferencesWindow, SIGNAL(highlightWindowChanged()), &flashHighlighter, SLOT(update()));
    connect(&findDialog, SIGNAL(singleWindowFound(Window*)), this, SLOT(locateWindowInTree(Window*)));
    connect(picker, SIGNAL(windowPicked(Window*)), this, SLOT(locateWindowInTree(Window*)));

    // Initialize current tree (will be read from settings if they exist)
    treeTabs->setCurrentIndex(1);
    currentTree = processWindowTree;

    // Read smart settings for window positions and other things
    readSmartSettings();

    desktopWindowTree->buildHeader();
    processWindowTree->buildHeader();
    refreshWindowTree();
}

MainWindow::~MainWindow() {
    delete picker;
}

/*------------------------------------------------------------------+
| Expands the items in the current tree to expose and highlight     |
| the item corresponding to the given window.                       |
+------------------------------------------------------------------*/
void MainWindow::locateWindowInTree(Window* window) {
    WindowItem* item = currentTree->findWindowItem(window);
    if (item) {
        item->expandAncestors();
        currentTree->setCurrentItem(item);
        if (isShiftDown()) viewWindowProperties();
        if (isCtrlDown())  viewWindowMessages();
    }
}

void MainWindow::readSmartSettings() {
    // If the settings don't exist, don't try to read them.
    // It will only mess up the window positions by defaulting to 0
    if (!Settings::isAppInstalled() ||
        !SmartSettings::subKeyExist("mainWindow"))
        return;

    SmartSettings settings;
    int x, y, width, height;

    // Main window geometry
    settings.setSubKey("mainWindow");
    bool shouldMaximize = settings.read<int>("isMaximized");
    x = settings.read<int>("x");
    y = settings.read<int>("y");
    width = settings.read<int>("width");
    height = settings.read<int>("height");
    move(x, y);
    resize(width, height);
    if (shouldMaximize)
        showMaximized();

    // Dock widgets
    settings.setSubKey("mainWindow.treeDock");
    bool isFloating;
    Qt::DockWidgetArea area;
    isFloating = settings.read<bool>("isFloating");
    area = static_cast<Qt::DockWidgetArea>(settings.read<int>("area"));
    if (isFloating)
        treeDock->setFloating(true);
    else
        addDockWidget(area, treeDock);
    x = settings.read<int>("x");
    y = settings.read<int>("y");
    width = settings.read<int>("width");
    height = settings.read<int>("height");
    treeDock->move(x, y);
    treeDock->resize(width, height);
    settings.setSubKey("mainWindow.statusDock");
    isFloating = settings.read<bool>("isFloating");
    area = static_cast<Qt::DockWidgetArea>(settings.read<int>("area"));
    if (isFloating)
        statusDock->setFloating(true);
    else
        addDockWidget(area, statusDock);
    x = settings.read<int>("x");
    y = settings.read<int>("y");
    width = settings.read<int>("width");
    height = settings.read<int>("height");
    statusDock->move(x, y);
    statusDock->resize(width, height);
    settings.setSubKey("mainWindow.treeTabs");
    int treeTabIndex = settings.read<int>("currentIndex");
    treeTabs->setCurrentIndex(treeTabIndex);
    currentTree = (treeTabIndex == 0 ? desktopWindowTree : processWindowTree);
}

void MainWindow::writeSmartSettings() {
    if (!Settings::isAppInstalled()) return;
    SmartSettings settings;

    // Main window geometry
    settings.setSubKey("mainWindow");
    settings.write<bool>("isMaximized", isMaximized());
    if (!isMaximized()) {   // Only remember un-maximised pos
        settings.writeWindowPos("x", x());
        settings.writeWindowPos("y", y());
        settings.writeWindowPos("width", width());
        settings.writeWindowPos("height", height());
    }

    // Dock widgets
    settings.setSubKey("mainWindow.treeDock");
    Qt::DockWidgetArea area;
    settings.write<bool>("isFloating", treeDock->isFloating());
    area = dockWidgetArea(treeDock);
    if (!treeDock->isFloating() && area != Qt::NoDockWidgetArea) {
        // Only remember dock area if docked
        settings.write<int>("area", static_cast<int>(area));
    }
    settings.writeWindowPos("x", treeDock->x());
    settings.writeWindowPos("y", treeDock->y());
    settings.writeWindowPos("width", treeDock->width());
    settings.writeWindowPos(".height", treeDock->height());
    settings.setSubKey("mainWindow.statusDock");
    settings.write<bool>("isFloating", statusDock->isFloating());
    area = dockWidgetArea(statusDock);
    if (!statusDock->isFloating() && area != Qt::NoDockWidgetArea) {
        // Only remember dock area if docked
        settings.write<int>("area", static_cast<int>(area));
    }
    settings.writeWindowPos("x", statusDock->x());
    settings.writeWindowPos("y", statusDock->y());
    settings.writeWindowPos("width", statusDock->width());
    settings.writeWindowPos("height", statusDock->height());
    settings.setSubKey("mainWindow.treeTabs");
    settings.write<int>("currentIndex", treeTabs->currentIndex());
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

    WindowManager::current()->refreshAllWindows();
    // TODO: Optimisation. Could only build current tree and set a flag
    // in WindowTree saying that it's built, and clear that flag in the
    // other tree. Then build other on change event if flag is not set
    desktopWindowTree->build();
    processWindowTree->build();
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

/*------------------------------------------------------------------+
| Adds the window to the MDI area and sets it's initial position    |
| and size. The size will be about 80% of the MDI area's smallest   |
| dimension and position it to ensure that it fits in view.         |
+------------------------------------------------------------------*/
void addWindowInMDI(QWidget* window, QMdiArea* mdiArea) {
    QMdiSubWindow* subWindow = mdiArea->addSubWindow(window);
    int minDim = qMin(mdiArea->size().width(), mdiArea->size().height());
    int width = qMin((int)(minDim * 0.90f), 600);
    int height = qMin((int)(minDim * 0.80f), 500);
    int x = rand(mdiArea->size().width() - width);
    int y = rand(mdiArea->size().height() - height);
    subWindow->setGeometry(x, y, width, height);
}

/*------------------------------------------------------------------+
| Creates a new property window and adds it to the MDI area.        |
+------------------------------------------------------------------*/
void MainWindow::viewWindowProperties() {
    PropertiesWindow* propertiesWindow = new PropertiesWindow(selectedWindow);
    propertiesWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(propertiesWindow, SIGNAL(locateWindow(Window*)), this, SLOT(locateWindowInTree(Window*)));
    connect(selectedWindow, SIGNAL(updated()), propertiesWindow, SLOT(update()));

    addWindowInMDI(propertiesWindow, mdiArea);
    propertiesWindow->show();
}

/*------------------------------------------------------------------+
| Creates a new message window and adds it to the MDI area.         |
| Also starts monitoring messages for the window.                   |
+------------------------------------------------------------------*/
void MainWindow::viewWindowMessages() {
    MessagesWindow* messagesWindow = new MessagesWindow(selectedWindow);
    messagesWindow->setAttribute(Qt::WA_DeleteOnClose);

    addWindowInMDI(messagesWindow, mdiArea);
    messagesWindow->show();
}

/*------------------------------------------------------------------+
| Opens a property dialog on the selected window.                   |
+------------------------------------------------------------------*/
void MainWindow::setWindowProperties() {
    SetPropertiesDialog* dialog = new SetPropertiesDialog(selectedWindow, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->showAtTab(0);
}

/*------------------------------------------------------------------+
| Opens a property dialog on the selected window and sets it to     |
| show the "window style" tab.                                      |
+------------------------------------------------------------------*/
void MainWindow::setWindowStyles() {
    SetPropertiesDialog* dialog = new SetPropertiesDialog(selectedWindow, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->showAtTab(1);
}

void MainWindow::actionShowWindow() {
    if (selectedWindow) selectedWindow->show();
}

void MainWindow::actionHideWindow() {
    if (selectedWindow) selectedWindow->hide();
}

void MainWindow::actionFlashWindow() {
    if (selectedWindow) flashHighlighter.flash(selectedWindow);
}

void MainWindow::actionCloseWindow() {
    if (selectedWindow) selectedWindow->close();
}

void MainWindow::showAboutDialog() {
    AboutDialog(this).exec();
}

void MainWindow::showEvent(QShowEvent*) {
    // Restore the size limitations that were set in constructor
    treeDock->setMinimumWidth(0);
    statusDock->setMaximumHeight(1000);
}

void MainWindow::closeEvent(QCloseEvent*) {
    writeSmartSettings();
    QApplication::quit();
}