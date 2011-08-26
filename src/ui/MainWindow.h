/////////////////////////////////////////////////////////////////////
// File: MainWindow.h                                              //
// Date: 15/2/10                                                   //
// Desc: The main UI window which is shown when the app starts.    //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2011 XTAL256

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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "window_detective/include.h"
#include "ui/forms/ui_MainWindow.h"
#include "window_detective/Logger.h"
#include "inspector/inspector.h"
#include "PreferencesWindow.h"
#include "FindDialog.h"
#include "SystemInfoViewer.h"
#include "AboutDialog.h"
#include "property_pages/PropertiesWindow.h"
#include "MessagesWindow.h"
#include "SetPropertiesDialog.h"
#include "custom_widgets/WindowPicker.h"
#include "custom_widgets/BalloonTip.h"
using namespace inspector;

#define MESSAGE_TIMEOUT       2000  //ms
#define TIP_TIMEOUT           10000
#define STATUS_ICON_TIMEOUT   30000
#define AUTO_SCROLL_PADDING   50

class MainWindow : public QMainWindow, private Ui::MainWindow, public LogListener {
    Q_OBJECT
private:
    WindowPicker* picker;
    QMenu windowMenu, processMenu;
    PreferencesWindow* preferencesWindow;
    FindDialog* findDialog;
    SystemInfoViewer* systemInfoDialog;
    AboutDialog* aboutDialog;
    QToolButton logButton;
    BalloonTip notificationTip;
    QTimer notificationTimer;
    QSignalMapper* mdiWindowMapper;
    bool isFirstTimeShow;      // For lazy-initializing stuff when window is opened

public:
    MainWindow(QMainWindow* parent = 0);
    ~MainWindow();

    void readSmartSettings();
    void writeSmartSettings();
    PreferencesWindow* getPreferencesWindow();
    FindDialog* getFindDialog();
    SystemInfoViewer* getSystemInfoDialog();
private:
    void buildTreeMenus();
    void addMdiWindow(QWidget* widget);
    QDockWidget* createLogWidget();
    void logAdded(Log* log);
    void logRemoved(Log*) {}
    void addLogToList(Log* log);
    void displayLogNotification(Log* log);
    void openDialog(QDialog* dialog);
protected:
    void showEvent(QShowEvent*);
    void moveEvent(QMoveEvent*);
    void resizeEvent(QMoveEvent*);
    void closeEvent(QCloseEvent*);
public slots:
    void refreshWindowTree();
    void openPreferences();
    void openFindDialog();
    void openSystemInfoDialog();
    void treeViewChanged(int index);
    void showTreeMenu(const QPoint& pos);
    void updateMdiMenu();
    void setActiveMdiWindow(QWidget* window);
    void stayOnTopChanged(bool shouldStayOnTop);
    void locateWindowInTree(Window*);
    void viewWindowProperties(QList<Window*>);
    void viewWindowMessages(QList<Window*>);
    void editWindowProperties(Window*);
    void editWindowStyles(Window*);
    void showLogs();
    void notificationTimeout();
    void showAboutDialog();
    void launchHelp();
};

#endif   // MAIN_WINDOW_H