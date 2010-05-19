/////////////////////////////////////////////////////////////////////
// File: MainWindow.h                                              //
// Date: 15/2/10                                                   //
// Desc: The main UI window which is shown when the app starts.    //
/////////////////////////////////////////////////////////////////////

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "ui_MainWindow.h"
#include "PreferencesWindow.h"
#include "FindDialog.h"
#include "PropertiesWindow.h"
#include "MessagesWindow.h"
#include "SetPropertiesDialog.h"
#include "WindowPicker.h"
using namespace inspector;

class MainWindow : public QMainWindow, private Ui::MainWindow {
    Q_OBJECT
private:
    WindowTree* currentTree;
    Window* selectedWindow;
    WindowPicker* picker;
    HighlightWindow flashHighlighter;
    PreferencesWindow preferencesWindow;
    FindDialog findDialog;

public:
    MainWindow(QMainWindow* parent = 0);
    ~MainWindow();

    void setupWindowMenu(Window* window);
    void openWindowList(QList<Window*> windows);
protected:
    void showEvent(QShowEvent* e);
    void closeEvent(QCloseEvent* e);
private slots:
    void refreshWindowTree();
    void openPreferences();
    void openFindDialog();
    void selectedWindowChanged();
    void treeTabChanged(int tabIndex);
    void showTreeContextMenu(const QPoint& pos);
    void locateWindowInTree(Window*);
    void windowsFound(QList<Window*>);
    void expandTreeItem();
    void viewWindowProperties();
    void setWindowProperties();
    void viewWindowMessages();
    void setWindowStyles();
    void actionShowWindow();
    void actionHideWindow();
    void actionFlashWindow();
    void actionCloseWindow();
};

#endif   // MAIN_WINDOW_H