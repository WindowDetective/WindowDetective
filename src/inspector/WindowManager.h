/////////////////////////////////////////////////////////////////////
// File: WindowManager.h                                           //
// Date: 20/2/10                                                   //
// Desc: Maintains a list of all windows and provides              //
//   functionality to search for a window and other things.        //
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

#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "inspector.h"
#include "SearchCriteria.h"

namespace inspector {

class WindowManager : public QObject {
    Q_OBJECT
private:
    static WindowManager* Current;   // Singleton instance
public:
    static void initialize();
    static WindowManager* current() { return Current; }
    static bool isOwnWindow(HWND handle);

    QList<Window*> allWindows;
    QList<Process*> allProcesses;

    WindowManager();
    ~WindowManager() {}

    void refreshAllWindows();
    Window* createWindow(HWND handle);
    Window* addWindow(HWND handle);
    void removeWindow(HWND handle);
    void removeWindow(Window* window);
    WindowClass* getWindowClassNamed(String name);
    Process* addProcess(uint pid);
    void removeProcess(Process* process);
    Window* find(HWND handle);
    Window* findParent(Window* window);
    WindowList findChildren(Window* window);
    Process* findProcess(uint pid);
    WindowList find(const SearchCriteria& criteria);
    Window* getDesktopWindow();
    Window* getWindowAt(const QPoint& p);
    WindowStyleList parseStyle(Window* window, DWORD styleBits, bool isExtended);
    uint styleBits(WindowStyleList stylesList);
    WindowStyleList getValidStandardStylesFor(Window*);
    WindowStyleList getValidExtendedStylesFor(Window*);
    WindowStyle* getStyleNamed(const String& name);
signals:
    void windowAdded(Window* window);
    void windowRemoved(Window* window);
    void processAdded(Process* process);
    void processRemoved(Process* process);
private:
    // The callback function to enumerate all child windows
    static BOOL CALLBACK enumChildWindows(HWND hwndChild, LPARAM lParam);
};

};   // namespace inspector

#endif   // WINDOW_MANAGER_H
