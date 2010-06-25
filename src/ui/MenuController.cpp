/////////////////////////////////////////////////////////////////////
// File: MenuController.cpp                                        //
// Date: 21/6/10                                                   //
// Desc: Creates menus for commands on window and process objects. //
//   These menus are used for context/popup menus on tree items.   //
/////////////////////////////////////////////////////////////////////

#include "MenuController.h"


/**********************/
/*** MenuController ***/
/**********************/

MenuController::MenuController() :
    menu(new QMenu()) {
}

MenuController::~MenuController() {
    delete menu;
}

/*------------------------------------------------------------------+
| Helper method to add a menu item to the given menu. The menu item |
| is a QAction which has a name and an optional icon.               |
+------------------------------------------------------------------*/
void MenuController::addMenuItem(QMenu* menu, String name,
                                 char* shortcut, char* iconFileName) {
    QAction* action = new QAction(NULL);
    action->setText(TR(name.toAscii()));
    if (shortcut) {
        action->setShortcut(QKeySequence(shortcut));
    }
    if (iconFileName) {
        QIcon icon;
        icon.addFile(QString::fromUtf8(iconFileName), QSize(),
                     QIcon::Normal, QIcon::Off);
        action->setIcon(icon);
    }
    menu->addAction(action);
}

void MenuController::popupAt(const QPoint& pos) {
    menu->exec(pos);
}


/****************************/
/*** WindowMenuController ***/
/****************************/

WindowMenuController::WindowMenuController() :
    MenuController() {
    buildMenu();
}

void WindowMenuController::buildMenu() {
    menu->setObjectName(QString::fromUtf8("windowMenu"));

    // TODO: Add icons, e.g. ":/img/blah.png"
    addMenuItem(menu, "View &Properties", "Ctrl+P");
    addMenuItem(menu, "Se&t Properties...");
    addMenuItem(menu, "&Messages...", "Ctrl+M");
    menu->addSeparator();
    addMenuItem(menu, "&Expand All");
    menu->addSeparator();
    addMenuItem(menu, "Edit Window &Styles");
    menu->addSeparator();
    addMenuItem(menu, "&Flash");
    addMenuItem(menu, "&Show");
    addMenuItem(menu, "&Hide");
    menu->addSeparator();
    addMenuItem(menu, "&Close");

    menu->setTitle(TR("&Window"));
}


/*****************************/
/*** ProcessMenuController ***/
/*****************************/

ProcessMenuController::ProcessMenuController() :
    MenuController() {
    buildMenu();
}

void ProcessMenuController::buildMenu() {
    menu->setObjectName(QString::fromUtf8("processMenu"));

    addMenuItem(menu, "&Expand All");

    menu->setTitle(TR("Process"));
}