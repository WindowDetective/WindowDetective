/////////////////////////////////////////////////////////////////////
// File: MenuController.h                                          //
// Date: 21/6/10                                                   //
// Desc: Creates menus for commands on window and process objects. //
//   These menus are used for context/popup menus on tree items.   //
/////////////////////////////////////////////////////////////////////

#ifndef MENU_CONTROLLER_H
#define MENU_CONTROLLER_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
using namespace inspector;

class MenuController {
protected:
    QMenu* menu;
public:
    MenuController();
    ~MenuController();

    virtual void buildMenu() = 0;
    void addMenuItem(QMenu* menu, String name,
                     char* shortcut = NULL, char* iconFileName = NULL);
    void popupAt(const QPoint& pos);
};

class WindowMenuController : public MenuController {
public:
    WindowMenuController();
    void buildMenu();
    QMenu menuForWindow(Window* window);
};

class ProcessMenuController : public MenuController {
public:
    ProcessMenuController();
    void buildMenu();
    QMenu menuForProcess(Process* process);
};

#endif   // MENU_CONTROLLER_H