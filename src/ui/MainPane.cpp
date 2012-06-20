/////////////////////////////////////////////////////////////////////
// File: MainPane.h                                                //
// Date: 2010-02-15                                                //
// Desc: The main UI window which is shown when the app starts.    //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2012 XTAL256

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

#include "MainPane.hpp"
#include "window_detective/main.h"
#include "inspector/WindowManager.hpp"
#include "inspector/MessageHandler.h"
#include "window_detective/Settings.h"
#include "custom_widgets/TreeItem.hpp"
#include "ActionManager.h"


MainPane::MainPane(QMainWindow *parent) :
    QMainWindow(parent),
    isFirstTimeShow(true),
    preferencesPane(NULL),
    findDialog(NULL),
    systemInfoDialog(NULL),
    aboutDialog(NULL),
    logButton(),
    notificationTip(&logButton),
    notificationTimer() {

    if (Settings::stayOnTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    }
    setupUi(this);

    picker = new WindowPicker(pickerToolBar, this);
    pickerToolBar->addWidget(picker);

    windowTree->setContextMenuPolicy(Qt::CustomContextMenu);

    // Setup status bar and "show logs" button
    logButton.setAutoRaise(true);
    // None of this works, either status bar is too big or button is too small
    //setMaximumHeight(22);
    //logButton.setMaximumSize(16, 16);
    //setContentsMargins...
    logButton.setToolButtonStyle(Qt::ToolButtonIconOnly);
    logButton.setIcon(QIcon(":/img/log_status.png"));
    logButton.setIconSize(QSize(16, 16));
    statusBar()->addPermanentWidget(&logButton);
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

    // Action events (menu or toolbar)
    connect(actnPreferences, SIGNAL(triggered()), this, SLOT(openPreferences()));
    connect(actnFind, SIGNAL(triggered()), this, SLOT(openFindDialog()));
    connect(actnSystemInfo, SIGNAL(triggered()), this, SLOT(openSystemInfoDialog()));
    connect(actnRefresh, SIGNAL(triggered()), this, SLOT(refreshWindowTree()));
    connect(actnHelp, SIGNAL(triggered()), this, SLOT(launchHelp()));
    connect(actnAbout, SIGNAL(triggered()), this, SLOT(showAboutDialog()));
    connect(menuWindows, SIGNAL(aboutToShow()), this, SLOT(updateMdiMenu()));

    // Other events
    connect(cbTreeView, SIGNAL(currentIndexChanged(int)), this, SLOT(treeViewChanged(int)));
    connect(windowTree, SIGNAL(currentItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)), this, SLOT(treeItemChanged(QTreeWidgetItem*,QTreeWidgetItem*)));
    connect(windowTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(showTreeMenu(const QPoint&)));
    connect(picker, SIGNAL(windowPicked(Window*)), this, SLOT(locateWindowInTree(Window*)));
    connect(&notificationTimer, SIGNAL(timeout()), this, SLOT(notificationTimeout()));
    connect(&logButton, SIGNAL(clicked()), this, SLOT(showLogs()));
    Logger::current().setListener(this); // Start listening for new logs

    readSmartSettings();
    buildTreeMenus();
}

MainPane::~MainPane() {
    delete picker;
    delete mdiWindowMapper;
    if (preferencesPane) delete preferencesPane;
    if (findDialog) delete findDialog;
    if (systemInfoDialog) delete systemInfoDialog;
    Logger::current().removeListener();
}

void MainPane::readSmartSettings() {
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

void MainPane::writeSmartSettings() {
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

void MainPane::buildTreeMenus() {
    QList<ActionType> windowMenuActions, processMenuActions;

    windowMenuActions
        << ActionViewProperties
        << ActionEditProperties
        << ActionViewMessages
        << Separator
        << ActionExpandAll
        << Separator
        << ActionEditStyles
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

/*--------------------------------------------------------------------------+
| Adds the window to the MDI area and sets it's initial position            |
+--------------------------------------------------------------------------*/
void MainPane::addMdiWindow(QWidget* widget) {
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

/*--------------------------------------------------------------------------+
| Functions to lazy-initialize dialogs.                                     |
+--------------------------------------------------------------------------*/
PreferencesPane* MainPane::getPreferencesPane() {
    if (!preferencesPane) {
        preferencesPane = new PreferencesPane();
        preferencesPane->move(x() + (width()  - preferencesPane->width())  / 2,
                                y() + (height() - preferencesPane->height()) / 2);
        connect(preferencesPane, SIGNAL(highlightWindowChanged()), &picker->highlighter, SLOT(update()));
        connect(preferencesPane, SIGNAL(highlightWindowChanged()), &Window::flashHighlighter, SLOT(update()));
        connect(preferencesPane, SIGNAL(stayOnTopChanged(bool)), this, SLOT(stayOnTopChanged(bool)));
    }
    return preferencesPane;
}

FindDialog* MainPane::getFindDialog() {
    if (!findDialog) {
        findDialog = new FindDialog(this);
        connect(findDialog, SIGNAL(singleWindowFound(Window*)), this, SLOT(locateWindowInTree(Window*)));
    }
    return findDialog;
}

SystemInfoViewer* MainPane::getSystemInfoDialog() {
    if (!systemInfoDialog) systemInfoDialog = new SystemInfoViewer(this);
    return systemInfoDialog;
}

/*--------------------------------------------------------------------------+
| Opens the given dialog or brings it to the front if it is already open.   |
+--------------------------------------------------------------------------*/
void MainPane::openDialog(QDialog* dialog) {
    if (dialog->isVisible()) {
        dialog->activateWindow();
        dialog->raise();
    }
    else {
        dialog->show();
    }
}

void MainPane::showEvent(QShowEvent*) {
    if (isFirstTimeShow) {

        // Remove the size limitations that were set in constructor
        treeDock->setMinimumWidth(0);

        // Add any existing logs
        if (logWidget->isVisible()) {
            logList->clear();
            QList<Log*> existingLogs = Logger::current().getLogs();
            QList<Log*>::const_iterator i;
            for (i = existingLogs.begin(); i != existingLogs.end(); ++i) {
                addLogToList(*i);
            }
        }

        // Update some stuff. Note: this is done here and not in the
        // constructor because they may rely on the UI being valid
        refreshWindowTree();
        cbTreeView->setCurrentIndex(windowTree->getType() == WindowTreeType ? 0 : 1);
        updateMdiMenu();

        isFirstTimeShow = false;
    }
}

void MainPane::moveEvent(QMoveEvent*) {
    if (notificationTip.isVisible()) {
        notificationTip.updatePosition();
    }
}

void MainPane::resizeEvent(QMoveEvent*) {
    if (notificationTip.isVisible()) {
        notificationTip.updatePosition();
    }
}

void MainPane::closeEvent(QCloseEvent*) {
    writeSmartSettings();
    QApplication::quit();
}

void MainPane::refreshWindowTree() {
    // Remember selected items' handles. We can't keep Window objects,
    // since we are destroying them
    QList<Window*> selectedWindows = windowTree->getSelectedWindows();
    QList<HWND> selectedHandles;
    for (QList<Window*>::const_iterator i = selectedWindows.begin();
         i != selectedWindows.end(); ++i) {
        selectedHandles.append((*i)->getHandle());
    }

    WindowManager::current().refreshAllWindows();
    windowTree->rebuild();

    // Re-select windows after rebuild
    Window* window = NULL;
    WindowItem* item = NULL;
    for (QList<HWND>::const_iterator i = selectedHandles.begin();
         i != selectedHandles.end(); ++i) {
        window = WindowManager::current().find(*i);
        if (window) {
            item = windowTree->findWindowItem(window);
            if (item) {
                item->expandAncestors();
                item->setSelected(true);
            }
        }
    }
    // And scroll to the first selected item (if any are selected)
    if (item) {
        windowTree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    }

    // Also update any open property windows with the new Window objects
    QList<QMdiSubWindow*> mdiWindows = mdiArea->subWindowList();
    foreach (QMdiSubWindow* each, mdiWindows) {
        each->widget()->setEnabled(false);
        /* TODO: Set model for either window and get them to update correctly
        if (MessagesPane* msgWindow = dynamic_cast<MessagesPane*>(each->widget())) {
        }
        else if (PropertiesPane* propWindow = dynamic_cast<PropertiesPane*>(each->widget())) {
        }*/
    }
}

void MainPane::openPreferences() {
    openDialog(getPreferencesPane());
}

void MainPane::openFindDialog() {
    openDialog(getFindDialog());
}

void MainPane::openSystemInfoDialog() {
    openDialog(getSystemInfoDialog());
}

/*--------------------------------------------------------------------------+
| Rebuilds the window tree when it changes, and re-select any               |
| selected items.                                                           |
+--------------------------------------------------------------------------*/
void MainPane::treeViewChanged(int index) {
    // Remember selected items
    QList<Window*> selectedWindows = windowTree->getSelectedWindows();

    windowTree->rebuild(index == 0 ? WindowTreeType : ProcessTreeType);

    // Re-select windows after rebuild
    QList<Window*>::const_iterator i;
    WindowItem* item = NULL;
    for (i = selectedWindows.begin(); i != selectedWindows.end(); ++i) {
        item = windowTree->findWindowItem(*i);
        if (item) {
            item->expandAncestors();
            item->setSelected(true);
        }
    }
    // And scroll to the first selected item (if any are selected)
    if (item) {
        windowTree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
    }
}

/*--------------------------------------------------------------------------+
| If Settings::openPropertiesOnSelect is true, show the properties          |
| of the selected item. Any other property windows will be closed.          |
+--------------------------------------------------------------------------*/
void MainPane::treeItemChanged(QTreeWidgetItem* current, QTreeWidgetItem* /*previous*/) {
    if (!Settings::openPropertiesOnSelect) return;

    mdiArea->closeAllSubWindows();
    WindowItem* windowItem = dynamic_cast<WindowItem*>(current);
    if (windowItem) {
        Window* window = windowItem->getWindow();
        if (window) {
            PropertiesPane* propWindow = viewWindowProperties(window);
            propWindow->showMaximized();
            windowTree->setFocus();
        }
    }
}

/*--------------------------------------------------------------------------+
| If one or more selected items are windows, the window menu is displayed   |
| and the action is executed on them.                                       |
| If the selected items are all processes, the process menu is displayed    |
| instead.                                                                  |
+--------------------------------------------------------------------------*/
void MainPane::showTreeMenu(const QPoint& /*unused*/) {
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
      case ActionEditProperties: {
          if (selectedWindows.isEmpty()) return;
          editWindowProperties(selectedWindows.first());
          break;
      }
      case ActionViewMessages: {
          viewWindowMessages(selectedWindows);
          break;
      }
      case ActionEditStyles: {
          if (selectedWindows.isEmpty()) return;
          editWindowStyles(selectedWindows.first());
          break;
      }
      case ActionFlashWindow: {
          if (selectedWindows.isEmpty()) return;
          selectedWindows.first()->flash();
          break;
      }
      case ActionShowWindow: {
          QList<Window*>::const_iterator i;
          for (i = selectedWindows.begin(); i != selectedWindows.end(); ++i) {
              (*i)->show();
          }
          break;
      }
      case ActionHideWindow: {
          QList<Window*>::const_iterator i;
          for (i = selectedWindows.begin(); i != selectedWindows.end(); ++i) {
              (*i)->hide();
          }
          break;
      }
      case ActionCloseWindow: {
          QList<Window*>::const_iterator i;
          for (i = selectedWindows.begin(); i != selectedWindows.end(); ++i) {
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

/*--------------------------------------------------------------------------+
| Updates the list of MDI windows in the menu.                              |
+--------------------------------------------------------------------------*/
void MainPane::updateMdiMenu() {
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
    for (i = windows.begin(); i != windows.end(); ++i) {
        QAction* action = menuWindows->addAction((*i)->windowTitle());
        action->setCheckable(true);
        action->setChecked(*i == mdiArea->activeSubWindow());
        connect(action, SIGNAL(triggered()), mdiWindowMapper, SLOT(map()));
        mdiWindowMapper->setMapping(action, *i);
    }
}

void MainPane::setActiveMdiWindow(QWidget* window) {
    if (!window) return;
    mdiArea->setActiveSubWindow(qobject_cast<QMdiSubWindow*>(window));
}

/*--------------------------------------------------------------------------+
| Set this window, and any dialogs it owns, to either stay on top           |
| of all other windows or not, depending on the given flag.                 |
| This event only happens when the preference is changed in the             |
| Preference Window. Normally, the "stay on top" flag is set when           |
| each respective window is created.                                        |
+--------------------------------------------------------------------------*/
void MainPane::stayOnTopChanged(bool shouldStayOnTop) {
    // Note: Setting window flags causes the window to be hidden, so we have to
    // show it again. This is because Qt needs to re-create the window, and i 
    // guess it can't be arsed showing it itself.
    if (shouldStayOnTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        show();
        if (findDialog) {
            findDialog->setWindowFlags(findDialog->windowFlags() | Qt::WindowStaysOnTopHint);
            findDialog->show();
        }
        if (systemInfoDialog) {
            systemInfoDialog->setWindowFlags(systemInfoDialog->windowFlags() | Qt::WindowStaysOnTopHint);
            systemInfoDialog->show();
        }
        if (preferencesPane) {
            preferencesPane->setWindowFlags(preferencesPane->windowFlags() | Qt::WindowStaysOnTopHint);
            preferencesPane->show();
        }
    }
    else {
        setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
        show();
        if (findDialog) {
            findDialog->setWindowFlags(findDialog->windowFlags() & ~Qt::WindowStaysOnTopHint);
            findDialog->show();
        }
        if (systemInfoDialog) {
            systemInfoDialog->setWindowFlags(systemInfoDialog->windowFlags() & ~Qt::WindowStaysOnTopHint);
            systemInfoDialog->show();
        }
        if (preferencesPane) {
            preferencesPane->setWindowFlags(preferencesPane->windowFlags() & ~Qt::WindowStaysOnTopHint);
            preferencesPane->show();
        }
    }
}

/*--------------------------------------------------------------------------+
| Expands the items in the current tree to expose and highlight             |
| the item corresponding to the given window.                               |
+--------------------------------------------------------------------------*/
void MainPane::locateWindowInTree(Window* window) {
    WindowItem* item = windowTree->findWindowItem(window);
    if (item) {
        item->expandAncestors();
        windowTree->setCurrentItem(item);
        windowTree->scrollToItem(item, QAbstractItemView::PositionAtCenter);
        windowTree->setFocus();
        if (isShiftDown()) viewWindowProperties(window);
        if (isCtrlDown())  viewWindowMessages(window);
    }
}

/*--------------------------------------------------------------------------+
| Creates a new property window and adds it to the MDI area.                |
+--------------------------------------------------------------------------*/
PropertiesPane* MainPane::viewWindowProperties(Window* window) {
    PropertiesPane* propertiesWindow = new PropertiesPane(window);
    propertiesWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(propertiesWindow, SIGNAL(locateWindow(Window*)), this, SLOT(locateWindowInTree(Window*)));
    connect(window, SIGNAL(updated()), propertiesWindow, SLOT(update()));

    addMdiWindow(propertiesWindow);
    propertiesWindow->show();

    return propertiesWindow;
}

void MainPane::viewWindowProperties(QList<Window*> windows) {
    QList<Window*>::const_iterator i;
    for (i = windows.begin(); i != windows.end(); ++i) {
        viewWindowProperties(*i);
    }
}

/*--------------------------------------------------------------------------+
| Creates a new message window and adds it to the MDI area.                 |
| Also starts monitoring messages for the window.                           |
+--------------------------------------------------------------------------*/
MessagesPane* MainPane::viewWindowMessages(Window* window) {
    MessagesPane* messagesWindow = new MessagesPane(window);
    messagesWindow->setAttribute(Qt::WA_DeleteOnClose);

    connect(messagesWindow, SIGNAL(locateWindow(Window*)), this, SLOT(locateWindowInTree(Window*)));

    addMdiWindow(messagesWindow);
    messagesWindow->show();

    return messagesWindow;
}

void MainPane::viewWindowMessages(QList<Window*> windows) {
    QList<Window*>::const_iterator i;
    for (i = windows.begin(); i != windows.end(); ++i) {
        viewWindowMessages(*i);
    }
}

/*--------------------------------------------------------------------------+
| Opens a property dialog on the given window.                              |
+--------------------------------------------------------------------------*/
// TODO: Maybe it could take multiple windows and open on each window one
//  after the other, or (even better) have the ability to set all at once
//  in the dialog. Fields specific to one window would be greyed out.
void MainPane::editWindowProperties(Window* window) {
    SetPropertiesDialog* dialog = new SetPropertiesDialog(window, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->showAtTab(0);
}

/*--------------------------------------------------------------------------+
| Opens a property dialog on the given window and sets it to  show          |
| the "window style" tab.                                                   |
+--------------------------------------------------------------------------*/
void MainPane::editWindowStyles(Window* window) {
    SetPropertiesDialog* dialog = new SetPropertiesDialog(window, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->showAtTab(1);
}

/*--------------------------------------------------------------------------+
| A log was added. If the logs list is visible, show it in that,            |
| otherwise show it in the status bar.                                      |
+--------------------------------------------------------------------------*/
void MainPane::logAdded(Log* log) {
    if (logWidget->isVisible()) {
        addLogToList(log);
    }
    else {
        displayLogNotification(log);
    }
}

/*--------------------------------------------------------------------------+
| Adds the log message to the logs list.                                    |
+--------------------------------------------------------------------------*/
void MainPane::addLogToList(Log* log) {
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

/*--------------------------------------------------------------------------+
| Display a notification in the status bar. If the log is a warning         |
| or error, a balloon tooltip will be displayed, otherwise the              |
| message will just be shown in the status bar.                             |
+--------------------------------------------------------------------------*/
void MainPane::displayLogNotification(Log* log) {
    LogLevel level = log->getLevel();
    if (level == InfoLevel) {
        // Show info messages in status bar
        logButton.setIcon(QIcon(":/img/info.png"));
        statusBar()->showMessage(log->getMessage().simplified(), MESSAGE_TIMEOUT);
        notificationTimer.start(MESSAGE_TIMEOUT);
    }
    if (level == WarnLevel || level == ErrorLevel) {
        // Warnings and errors will display a balloon tooltip
        logButton.setIcon(level == WarnLevel ? QIcon(":/img/warning.png") : QIcon(":/img/error.png"));
        if (Settings::enableBalloonNotifications) {
            notificationTip.showMessage(log->getMessage(), TIP_TIMEOUT);
        }
        notificationTimer.start(STATUS_ICON_TIMEOUT);
    }
}

/*--------------------------------------------------------------------------+
| Display the log widget visible and un-docked.                             |
+--------------------------------------------------------------------------*/
void MainPane::showLogs() {
    if (logWidget->isVisible()) return;

    // Add any existing logs
    logList->clear();
    QList<Log*> existingLogs = Logger::current().getLogs();
    QList<Log*>::const_iterator i;
    for (i = existingLogs.begin(); i != existingLogs.end(); ++i) {
        addLogToList(*i);
    }

    logButton.setIcon(QIcon(":/img/log_status.png"));
    logWidget->show();
    logWidget->setFloating(true);
    logWidget->resize(600, 400);
    logWidget->move(x() + (width()  - logWidget->width())  / 2,
                    y() + (height() - logWidget->height()) / 2);
}

/*--------------------------------------------------------------------------+
| The notification timer has gone off. Restore anything that was            |
| changed for the notification (e.g. status icon).                          |
+--------------------------------------------------------------------------*/
void MainPane::notificationTimeout() {
    logButton.setIcon(QIcon(":/img/log_status.png"));
}

/*--------------------------------------------------------------------------+
| Opens the "About" dialog. Note: This cannot be shown modal,               |
| because of the magnifying glass "easter-egg" window.                      |
+--------------------------------------------------------------------------*/
void MainPane::showAboutDialog() {
    if (!aboutDialog) aboutDialog = new AboutDialog(this);
    aboutDialog->move(x() + (width()  - aboutDialog->width())  / 2,
                      y() + (height() - aboutDialog->height()) / 2);
    openDialog(aboutDialog);
}

/*--------------------------------------------------------------------------+
| Opens the main help page in the external browser.                         |
+--------------------------------------------------------------------------*/
void MainPane::launchHelp() {
    QUrl helpFile("file:///" + appPath() + "/help/index.html");
    QDesktopServices::openUrl(helpFile);
}
