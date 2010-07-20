/////////////////////////////////////////////////////////////////////
// File: ActionManager.h                                           //
// Date: 21/6/10                                                   //
// Desc: Creates actions for menu items and toolbar buttons.       //
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

#ifndef ACTION_MANAGER_H
#define ACTION_MANAGER_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
using namespace inspector;

/* TODO: 
  Make an ActionManager class instead, which will hold all
  QAction objects for each menu item. Then each window/pane will
  have a method where it builds it's menu by specifying a list
  of actions to use. Not sure how to connect each action to a
  handler, though, as each window may handle it separately.

class ActionManager
  * keeps a list of QActions, created at startup


class WindowTree, SearchResultsWindow
  * has a QMenu for window and process items
  * on right-click, shows the menu for the selected item/s

* Main window needs to somehow connect itself to an action's triggered
  signals. But it won't be able to access the action objects.

*/
class ActionManager {
protected:
    static QMap<String, QAction*> actions;
    static void addAction(String id, String name,
            char* shortcut = NULL, char* iconFileName = NULL);
public:
    static void initialize();
    static QAction* getAction(String id);
    static QAction* cloneAction(String id);
};

#endif   // ACTION_MANAGER_H