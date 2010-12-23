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
    isFirstTimeShow(true),
    findDialog(this),
    preferencesWindow(),
    notificationTimer(),
    notificationTip() {
    setupUi(this);

    picker = new WindowPicker(pickerToolBar, this);
    pickerToolBar->addWidget(picker);

    windowTree->setContextMenuPolicy(Qt::CustomContextMenu);

    // Setup status bar and "show logs" button
    logButton = new QToolButton();
    logButton->setAutoRaise(true);
    // None of this works, either status bar is too big or button is too small
    //setMaximumHeight(22);
    //logButton->setMaximumSize(16, 16);
    //setContentsMargins...
    logButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
    logButton->setIcon(QIcon(":/img/log_status.png"));
    logButton->setIconSize(QSize(16, 16));
    statusBar()->addPermanentWidget(logButton);
    notificationTip.setOwner(logButton);
    logWidget->hide();
    notificationTimer.setSingleShot(true);

    // Since there doesn't seem to be any way to control the size of dock
    // widgets, we have to force a max/min size. These size limits are
    // removed when the window is shows, after the layout manager has sized them
    treeDock->setMinimumWidth(300);

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
    connect(cbTreeView, SIGNAL(currentIndexChanged(int)), this, SLOT(treeViewChanged(int)));
    connect(windowTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTreeMenu(const QPoint&)));
    connect(&preferencesWindow, SIGNAL(highlightWindowChanged()), &picker->highlighter, SLOT(update()));
    connect(&preferencesWindow, SIGNAL(highlightWindowChanged()), &Window::flashHighlighter, SLOT(update()));
    connect(&findDialog, SIGNAL(singleWindowFound(Window*)), this, SLOT(locateWindowInTree(Window*)));
    connect(picker, SIGNAL(windowPicked(Window*)), this, SLOT(locateWindowInTree(Window*)));
    connect(&notificationTimer, SIGNAL(timeout()), this, SLOT(notificationTimeout()));
    connect(logButton, SIGNAL(clicked()), this, SLOT(showLogs()));
    Logger::current()->setListener(this); // Start listening for new logs

    readSmartSettings();
    buildTreeMenus();
}

MainWindow::~MainWindow() {
    delete picker;
    delete mdiWindowMapper;
    Logger::current()->removeListener();
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
    TreeType treeType = static_cast<TreeType>(settings.read<int>("treeType"));
    windowTree->setType(treeType);

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
    settings.setSubKey("mainWindow.logWidget");
    isFloating = settings.read<bool>("isFloating");
    logWidget->setVisible(settings.read<bool>("isVisible"));
    area = static_cast<Qt::DockWidgetArea>(settings.read<int>("area"));
    if (isFloating) {
        logWidget->setFloating(true);
    }
    else {
        addDockWidget(area, logWidget);
    }
    x = settings.read<int>("x");
    y = settings.read<int>("y");
    width = settings.read<int>("width");
    height = settings.read<int>("height");
    logWidget->move(x, y);
    logWidget->resize(width, height);
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
    settings.write<int>("treeType", static_cast<int>(windowTree->getType()));

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
    settings.setSubKey("mainWindow.logWidget");
    settings.write<bool>("isVisible", logWidget->isVisible());
    settings.write<bool>("isFloating", logWidget->isFloating());
    area = dockWidgetArea(logWidget);
    if (!logWidget->isFloating() && area != Qt::NoDockWidgetArea) {
        // Only remember dock area if docked
        settings.write<int>("area", static_cast<int>(area));
    }
    settings.writeWindowPos("x", logWidget->x());
    settings.writeWindowPos("y", logWidget->y());
    settings.writeWindowPos("width", logWidget->width());
    settings.writeWindowPos("height", logWidget->height());
}


/**********************/
/*** Event handlers ***/
/**********************/

void MainWindow::showEvent(QShowEvent*) {
    if (isFirstTimeShow) isFirstTimeShow = false;
    else return;

    // Remove the size limitations that were set in constructor
    treeDock->setMinimumWidth(0);

    // Add any existing logs
    if (logWidget->isVisible()) {
        logList->clear();
        QList<Log*> existingLogs = Logger::current()->getLogs();
        QList<Log*>::const_iterator i;
        for (i = existingLogs.begin(); i != existingLogs.end(); i++) {
            addLogToList(*i);
        }
    }
    
    // Update some stuff. Note: this is done here and not in the
    // constructor because they may rely on the UI being valid
    refreshWindowTree();
    cbTreeView->setCurrentIndex(windowTree->getType() == WindowTreeType ? 0 : 1);
    updateMdiMenu();
}

void MainWindow::closeEvent(QCloseEvent*) {
    writeSmartSettings();
    QApplication::quit();
}

void MainWindow::refreshWindowTree() {
    // Rebuilding the tree will destroy all Window objects.
    // That will invalidate any property windows, so we disable them.
    QList<QMdiSubWindow*> mdiWindows = mdiArea->subWindowList();
    foreach (QMdiSubWindow* each, mdiWindows) {
        each->widget()->setEnabled(false);
    }

    WindowManager::current()->refreshAllWindows();
    windowTree->rebuild();
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

void MainWindow::treeViewChanged(int index) {
    // TODO: remember selected item and re-select it
    windowTree->rebuild(index == 0 ? WindowTreeType : ProcessTreeType);
}

/*------------------------------------------------------------------+
| If one or more selected items are windows, the window menu is     |
| displayed and the action is executed on them.                     |
| If the selected items are all processes, the process menu is      |
| displayed instead.                                                |
+------------------------------------------------------------------*/
void MainWindow::showTreeMenu(const QPoint& /*unused*/) {
    Action* action = NULL;

    QList<Window*> selectedWindows = windowTree->getSelectedWindows();
    if (!selectedWindows.isEmpty()) {
        action = dynamic_cast<Action*>(windowMenu.exec(QCursor::pos()));
    }
    else {
        action = dynamic_cast<Action*>(processMenu.exec(QCursor::pos()));
    }
    if (!action) return;   // User cancelled

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
          windowTree->expandSelected();
          break;
      }
    }
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
| Expands the items in the current tree to expose and highlight     |
| the item corresponding to the given window.                       |
+------------------------------------------------------------------*/
void MainWindow::locateWindowInTree(Window* window) {
    WindowItem* item = windowTree->findWindowItem(window);
    if (item) {
        item->expandAncestors();
        windowTree->setCurrentItem(item);
        QList<Window*> windows;
        windows.append(window);  // Only one item, but functions take a list
        if (isShiftDown()) viewWindowProperties(windows);
        if (isCtrlDown())  viewWindowMessages(windows);
    }
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
| A log was added. If the logs list is visible, show it in that,    |
| otherwise show it in the status bar.                              |
+------------------------------------------------------------------*/
void MainWindow::logAdded(Log* log) {
    if (logWidget->isVisible()) {
        addLogToList(log);
    }
    else {
        // TODO: Only if logButton is fully visible (i.e. not obscured by other
        // windows). Not sure how i will check for this.
        displayLogNotification(log);
    }
}

/*------------------------------------------------------------------+
| Adds the log message to the logs list.                            |
+------------------------------------------------------------------*/
void MainWindow::addLogToList(Log* log) {
    // "Abuse" the QTreeWidget by only using top-level items to make it
    // look like a list view with columns.
    QTreeWidgetItem* item = new QTreeWidgetItem(logList);

    String timeString = log->getTime().toString(Qt::SystemLocaleShortDate);
    item->setText(0, timeString);
    item->setText(1, log->levelName());
    item->setText(2, log->getMessage().simplified());

    // Set background colour based on log level
    QColor backgroundColour;
    switch (log->getLevel()) {
        case ErrorLevel: backgroundColour = QColor(255, 85, 85); break;
        case WarnLevel:  backgroundColour = QColor(255, 170, 85); break;
        case DebugLevel: backgroundColour = QColor(85, 170, 255); break;
        default:         backgroundColour = QColor(255, 255, 255); break;
    }
    item->setBackground(1, QBrush(backgroundColour));

    // Auto-scroll if necessary
    QScrollBar* sb = logList->verticalScrollBar();
    if (sb && sb->value() >= sb->maximum()-AUTO_SCROLL_PADDING)
        logList->scrollToBottom();
}

/*------------------------------------------------------------------+
| Display a notification in the status bar. If the log is a warning |
| or error, a balloon tooltip will be displayed, otherwise the      |
| message will just be shown in the status bar.                     |
+------------------------------------------------------------------*/
void MainWindow::displayLogNotification(Log* log) {
    switch (log->getLevel()) {
      case InfoLevel: {
         // Show info messages in status bar
         statusBar()->showMessage(log->getMessage().simplified(), MESSAGE_TIMEOUT);
         logButton->setIcon(QIcon(":/img/info.png"));
         notificationTimer.start(MESSAGE_TIMEOUT);
         break;
      }
      case WarnLevel: {
         // Warnings and errors will display a balloon tooltip
         notificationTip.showMessage(log->getMessage(), TIP_TIMEOUT);
         logButton->setIcon(QIcon(":/img/warning.png"));
         notificationTimer.start(STATUS_ICON_TIMEOUT);
         break;
      }
      case ErrorLevel: {
         notificationTip.showMessage(log->getMessage(), TIP_TIMEOUT);
         logButton->setIcon(QIcon(":/img/error.png"));
         notificationTimer.start(STATUS_ICON_TIMEOUT);
         break;
      }
    }
}

/*------------------------------------------------------------------+
| Display the log widget visible and un-docked.                     |
+------------------------------------------------------------------*/
void MainWindow::showLogs() {
    if (logWidget->isVisible()) return;

    // Add any existing logs
    logList->clear();
    QList<Log*> existingLogs = Logger::current()->getLogs();
    QList<Log*>::const_iterator i;
    for (i = existingLogs.begin(); i != existingLogs.end(); i++) {
        addLogToList(*i);
    }

    logButton->setIcon(QIcon(":/img/log_status.png"));
    logWidget->show();
    logWidget->setFloating(true);
    logWidget->move(x()+(width()-600)/2, y()+(height()-400)/2);
    logWidget->resize(600, 400);
}

/*------------------------------------------------------------------+
| The notification timer has gone off. Restore anything that was    |
| changed for the notification (e.g. status icon).                  |
+------------------------------------------------------------------*/
void MainWindow::notificationTimeout() {
    logButton->setIcon(QIcon(":/img/log_status.png"));
}

void MainWindow::showAboutDialog() {
    AboutDialog(this).exec();
}

/*------------------------------------------------------------------+
| Opens the main help page in the external browser.                 |
+------------------------------------------------------------------*/
void MainWindow::launchHelp() {
    QUrl helpFile("file:///" + appPath() + "/help/index.html");
    QDesktopServices::openUrl(helpFile);
}
