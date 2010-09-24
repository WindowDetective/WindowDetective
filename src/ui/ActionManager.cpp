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

QMap<ActionType,Action*> ActionManager::actions;

/*------------------------------------------------------------------+
| Creates and adds an action with the given id, name and function to|
| execute. An optional shortcut key and icon can also be provided.  |
+------------------------------------------------------------------*/
void ActionManager::addAction(ActionType id, String name,
                              char* shortcut, char* iconFileName) {
    Action* action = new Action(id);
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
    addAction(ActionViewProperties, TR("View &Properties"), "Ctrl+P");
    addAction(ActionSetProperties, TR("Se&t Properties..."));
    addAction(ActionViewMessages, TR("&Messages..."), "Ctrl+M");
    addAction(ActionSetStyles, TR("Edit Window &Styles"));
    addAction(ActionFlashWindow, TR("&Flash"));
    addAction(ActionShowWindow, TR("&Show"));
    addAction(ActionHideWindow, TR("&Hide"));
    addAction(ActionCloseWindow, TR("&Close"));
    addAction(ActionShowInTree, TR("Show in &Tree"));
    addAction(ActionExpandAll, TR("&Expand All"));
}

void ActionManager::fillMenu(QMenu& menu, QList<ActionType> actionIds) {
    QList<ActionType>::const_iterator i;
    for (i = actionIds.constBegin(); i != actionIds.constEnd(); i++) {
        if (*i == Separator) {
            menu.addSeparator();
        }
        else {
            menu.addAction(actions.value(*i));
        }
    }
}
