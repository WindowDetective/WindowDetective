/////////////////////////////////////////////////////////////////////
// File: MainWindow.h                                              //
// Date: 15/2/10                                                   //
// Desc: The main UI window which is shown when the app starts.    //
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

#include "MainWindow.h"
#include "AboutDialog.h"
#include "window_detective/main.h"
#include "inspector/WindowManager.h"
#include "inspector/MessageHandler.h"
#include "window_detective/Settings.h"
#include "custom_widgets/TreeItem.h"
#include "ActionManager.h"
using namespace inspector;

MainWindow::MainWindow(QMainWindow *parent) :
    QMainWindow(parent),
    findDialog(this),
    preferencesWindow() {
    setupUi(this);

    picker = new WindowPicker(pickerToolBar, this);
    pickerToolBar->addWidget(picker);

    desktopWindowTree->setContextMenuPolicy(Qt::CustomContextMenu);
    processWindowTree->setContextMenuPolicy(Qt::CustomContextMenu);

    // Since there doesn't seem to be any way to control the size of dock
    // widgets, we have to force a max/min size. These size limits are
    // removed when the window is shows, after the layout manager has sized them
    treeDock->setMinimumWidth(300);
    statusDock->setMaximumHeight(180);

    // MDI events
    mdiWindowMapper = new QSignalMapper(this);
    connect(mdiWindowMapper, SIGNAL(mapped(QWidget*)), this, SLOT(setActiveMdiWindow(QWidget*)));
    connect(mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateMdiMenu()));
    connect(actnCascade, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()));
    connect(actnTile, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()));
    connect(actnCloseAllMdi, SIGNAL(triggered()), mdiArea, SLOT(closeAllSubWindows()));

    // Menu events
    connect(actnPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
    connect(actnFind, SIGNAL(triggered()), this, SLOT(openFindDialog()));
    connect(actnRefresh, SIGNAL(triggered()), this, SLOT(refreshWindowTree()));
    connect(actnHelp, SIGNAL(triggered()), this, SLOT(launchHelp()));
    connect(actnAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    connect(menuWindows, SIGNAL(aboutToShow()), this, SLOT(updateMdiMenu()));

    // Other events
    connect(treeTabs, SIGNAL(currentChanged(int)), this, SLOT(treeTabChanged(int)));
    connect(desktopWindowTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showDesktopTreeMenu(const QPoint&)));
    connect(processWindowTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showProcessTreeMenu(const QPoint&)));
    connect(&preferencesWindow, SIGNAL(highlightWindowChanged()), &picker->highlighter, SLOT(update()));
    connect(&preferencesWindow, SIGNAL(highlightWindowChanged()), &Window::flashHighlighter, SLOT(update()));
    connect(&findDialog, SIGNAL(singleWindowFound(Window*)), this, SLOT(locateWindowInTree(Window*)));
    connect(picker, SIGNAL(windowPicked(Window*)), this, SLOT(locateWindowInTree(Window*)));

    // Initialize current tree (will be read from settings if they exist)
    treeTabs->setCurrentIndex(1);
    currentTree = processWindowTree;

    // Read smart settings for window positions and other things
    readSmartSettings();

    buildTreeMenus();
    desktopWindowTree->buildHeader();
    processWindowTree->buildHeader();
    refreshWindowTree();
    updateMdiMenu();
}

MainWindow::~MainWindow() {
    delete picker;
    delete mdiWindowMapper;
}

void MainWindow::buildTreeMenus() {
    QList<ActionType> windowMenuActions, processMenuActions;

    windowMenuActions
        << ActionViewProperties
        << ActionSetProperties
        << ActionViewMessages
        << Separator
        << ActionExpandAll
        << Separator
        << ActionSetStyles
        << Separator
        << ActionFlashWindow
        << ActionShowWindow
        << ActionHideWindow
        << Separator
        << ActionCloseWindow;

    processMenuActions
        << ActionExpandAll;

    ActionManager::fillMenu(windowMenu, windowMenuActions);
    ActionManager::fillMenu(processMenu, processMenuActions);
}

/*------------------------------------------------------------------+
| Expands the items in the current tree to expose and highlight     |
| the item corresponding to the given window.                       |
+------------------------------------------------------------------*/
// TODO: Put this (as well as view/set props) in ActionManager. Then instead of
// having other windows connect signals to this, they can just call it directly
void MainWindow::locateWindowInTree(Window* window) {
    WindowItem* item = currentTree->findWindowItem(window);
    if (item) {
        item->expandAncestors();
        currentTree->setCurrentItem(item);
        QList<Window*> windows;
        windows.append(window);  // Only one item, but functions take a list
        if (isShiftDown()) viewWindowProperties(windows);
        if (isCtrlDown())  viewWindowMessages(windows);
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
    if (isFloating) {
        treeDock->setFloating(true);
    }
    else {
        addDockWidget(area, treeDock);
    }
    x = settings.read<int>("x");
    y = settings.read<int>("y");
    width = settings.read<int>("width");
    height = settings.read<int>("height");
    treeDock->move(x, y);
    treeDock->resize(width, height);
    settings.setSubKey("mainWindow.statusDock");
    isFloating = settings.read<bool>("isFloating");
    area = static_cast<Qt::DockWidgetArea>(settings.read<int>("area"));
    if (isFloating) {
        statusDock->setFloating(true);
    }
    else {
        addDockWidget(area, statusDock);
    }
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

/*------------------------------------------------------------------+
| Opens the preferences dialog or brings it to the front if it is   |
| already open. Only one preferences dialog can be open at a time.  |
+------------------------------------------------------------------*/
void MainWindow::openPreferences() {
    if (preferencesWindow.isVisible()) {
        preferencesWindow.activateWindow();
        preferencesWindow.raise();
    }
    else {
        preferencesWindow.show();
    }
}

/*------------------------------------------------------------------+
| Opens the find dialog or brings it to the front if it is already  |
| open. Only one find dialog can be open at a time.                 |
+------------------------------------------------------------------*/
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

/*------------------------------------------------------------------+
| Displays the menu for the desktop window tree and executes the    |
| action on the selected window/s.                                  |
+------------------------------------------------------------------*/
void MainWindow::showDesktopTreeMenu(const QPoint& /*unused*/) {
    if (!currentTree) currentTree = desktopWindowTree;

    Action* action = dynamic_cast<Action*>(windowMenu.exec(QCursor::pos()));
    if (!action) return;      // User cancelled

    QList<Window*> selectedWindows = currentTree->getSelectedWindows();
    if (selectedWindows.isEmpty()) return; // Nothing selected

    switch (action->id) {
      case ActionViewProperties: {
          viewWindowProperties(selectedWindows);
          break;
      }
      case ActionSetProperties: {
          if (selectedWindows.isEmpty()) return;
          setWindowProperties(selectedWindows.first());
          break;
      }
      case ActionViewMessages: {
          viewWindowMessages(selectedWindows);
          break;
      }
      case ActionSetStyles: {
          if (selectedWindows.isEmpty()) return;
          setWindowStyles(selectedWindows.first());
          break;
      }
      case ActionFlashWindow: {
          if (selectedWindows.isEmpty()) return;
          selectedWindows.first()->flash();
          break;
      }
      case ActionShowWindow: {
          QList<Window*>::const_iterator i;
          for (i = selectedWindows.constBegin(); i != selectedWindows.constEnd(); i++) {
              (*i)->show();
          }
          break;
      }
      case ActionHideWindow: {
          QList<Window*>::const_iterator i;
          for (i = selectedWindows.constBegin(); i != selectedWindows.constEnd(); i++) {
              (*i)->hide();
          }
          break;
      }
      case ActionCloseWindow: {
          QList<Window*>::const_iterator i;
          for (i = selectedWindows.constBegin(); i != selectedWindows.constEnd(); i++) {
              (*i)->close();
          }
          break;
      }
      case ActionExpandAll: {
          currentTree->expandSelected();
          break;
      }
    }
}

/*------------------------------------------------------------------+
| If the selected items are all processes, the menu for the process |
| window tree is displayed and the action is executed on selected   |
| processes.                                                        |
| If one or more items are windows, the window menu is displayed    |
| instead (i.e. showDesktopTreeMenu is called).                     |
+------------------------------------------------------------------*/
void MainWindow::showProcessTreeMenu(const QPoint& pos) {
    if (!currentTree) currentTree = processWindowTree;

    // If there are window items selected, use desktop tree menu
    QList<Window*> selectedWindows = currentTree->getSelectedWindows();
    if (!selectedWindows.isEmpty())
        return showDesktopTreeMenu(pos);

    Action* action = dynamic_cast<Action*>(processMenu.exec(QCursor::pos()));
    if (!action) return;   // User cancelled

    // No need for this yet
    //Process* selectedProcess = getSelectedProcess();
    //if (!selectedProcess) return;

    switch (action->id) {
      case ActionExpandAll: {
          currentTree->expandSelected();
          break;
      }
    }
}

/*------------------------------------------------------------------+
| Adds the window to the MDI area and sets it's initial position    |
+------------------------------------------------------------------*/
void MainWindow::addMdiWindow(QWidget* widget) {
    QMdiSubWindow* subWindow = mdiArea->addSubWindow(widget);

    // The size should be about 80% of the MDI area's smallest dimension
    // Min size = 450x370, max size = 600x500.
    int minDim = qMin(mdiArea->size().width(), mdiArea->size().height());
    int width = qMax(450, qMin((int)(minDim * 0.90f), 600));
    int height = qMax(370, qMin((int)(minDim * 0.80f), 500));
    int x = rand(mdiArea->size().width() - width);
    int y = rand(mdiArea->size().height() - height);
    subWindow->setGeometry(x, y, width, height);
}

/*------------------------------------------------------------------+
| Updates the list of MDI windows in the menu.                      |
+------------------------------------------------------------------*/
void MainWindow::updateMdiMenu() {
    QList<QMdiSubWindow*> windows = mdiArea->subWindowList();
    QList<QMdiSubWindow*>::const_iterator i;

    menuWindows->clear();
    menuWindows->addAction(actnCascade);
    menuWindows->addAction(actnTile);
    if (windows.isEmpty()) {
        actnCascade->setEnabled(false);
        actnTile->setEnabled(false);
    }
    else {
        actnCascade->setEnabled(true);
        actnTile->setEnabled(true);
        menuWindows->addSeparator();
        menuWindows->addAction(actnCloseAllMdi);
    }
    for (i = windows.constBegin(); i != windows.constEnd(); i++) {
        QAction* action = menuWindows->addAction((*i)->windowTitle());
        action->setCheckable(true);
        action->setChecked(*i == mdiArea->activeSubWindow());
        connect(action, SIGNAL(triggered()), mdiWindowMapper, SLOT(map()));
        mdiWindowMapper->setMapping(action, *i);
    }
}

void MainWindow::setActiveMdiWindow(QWidget* window) {
    if (!window) return;
    mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

/*------------------------------------------------------------------+
| Creates a new property window and adds it to the MDI area.        |
+------------------------------------------------------------------*/
void MainWindow::viewWindowProperties(QList<Window*> windows) {
    QList<Window*>::const_iterator i;
    for (i = windows.constBegin(); i != windows.constEnd(); i++) {
        PropertiesWindow* propertiesWindow = new PropertiesWindow(*i);
        propertiesWindow->setAttribute(Qt::WA_DeleteOnClose);

        connect(propertiesWindow, SIGNAL(locateWindow(Window*)), this, SLOT(locateWindowInTree(Window*)));
        connect(*i, SIGNAL(updated()), propertiesWindow, SLOT(update()));

        addMdiWindow(propertiesWindow);
        propertiesWindow->show();
    }
}

/*------------------------------------------------------------------+
| Creates a new message window and adds it to the MDI area.         |
| Also starts monitoring messages for the window.                   |
+------------------------------------------------------------------*/
void MainWindow::viewWindowMessages(QList<Window*> windows) {
    QList<Window*>::const_iterator i;
    for (i = windows.constBegin(); i != windows.constEnd(); i++) {
        MessagesWindow* messagesWindow = new MessagesWindow(*i);
        messagesWindow->setAttribute(Qt::WA_DeleteOnClose);

        addMdiWindow(messagesWindow);
        messagesWindow->show();
    }
}

/*------------------------------------------------------------------+
| Opens a property dialog on the given window.                      |
+------------------------------------------------------------------*/
// TODO: Maybe it could take multiple windows and open on each window one
//  after the other, or (even better) have the ability to set all at once
//  in the dialog. Fields specific to one window would be greyed out. 
void MainWindow::setWindowProperties(Window* window) {
    SetPropertiesDialog* dialog = new SetPropertiesDialog(window, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->showAtTab(0);
}

/*------------------------------------------------------------------+
| Opens a property dialog on the given window and sets it to  show  |
| the "window style" tab.                                           |
+------------------------------------------------------------------*/
void MainWindow::setWindowStyles(Window* window) {
    SetPropertiesDialog* dialog = new SetPropertiesDialog(window, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->showAtTab(1);
}

/*------------------------------------------------------------------+
| Opens the main help page in the external browser.                 |
+------------------------------------------------------------------*/
void MainWindow::launchHelp() {
    QUrl helpFile("file:///" + appPath() + "/help/index.html");
    QDesktopServices::openUrl(helpFile);
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