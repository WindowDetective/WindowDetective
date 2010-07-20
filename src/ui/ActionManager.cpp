/////////////////////////////////////////////////////////////////////
// File: ActionManager.cpp                                         //
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

#include "ActionManager.h"


QMap<String, QAction*> ActionManager::actions;

/*------------------------------------------------------------------+
| Adds an action to the collection of actions. It has an id and     |
| name as well as an optional shortcut key and icon.                |
+------------------------------------------------------------------*/
void ActionManager::addAction(String id, String name,
                              char* shortcut, char* iconFileName) {
    QAction* action = new QAction(NULL);
    action->setText(name);
    if (shortcut) {
        action->setShortcut(QKeySequence(shortcut));
    }
    if (iconFileName) {
        QIcon icon;
        icon.addFile(QString::fromUtf8(iconFileName), QSize(),
                     QIcon::Normal, QIcon::Off);
        action->setIcon(icon);
    }
    actions.insert(id, action);
}

void ActionManager::initialize() {
    // TODO: Add icons, e.g. ":/img/blah.png"
    addAction("viewProperties", TR("View &Properties"), "Ctrl+P");
    addAction("setProperties", TR("Se&t Properties..."));
    addAction("viewMessages", TR("&Messages..."), "Ctrl+M");
    addAction("setStyles", TR("Edit Window &Styles"));
    addAction("flashWindow", TR("&Flash"));
    addAction("showWindow", TR("&Show"));
    addAction("hideWindow", TR("&Hide"));
    addAction("closeWindow", TR("&Close"));
    addAction("expandAll", TR("&Expand All"));
}

QAction* ActionManager::getAction(String id) {
    if (!actions.contains(id))
        return NULL;
    return actions.value(id);
}

QAction* ActionManager::cloneAction(String id) {
    return NULL;
}
