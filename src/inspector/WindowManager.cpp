/////////////////////////////////////////////////////////////////////
// File: WindowManager.cpp                                         //
// Date: 20/2/10                                                   //
// Desc: Maintains a list of all windows and provides              //
//   functionality to search for a window and other things.        //
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

#include "window_detective/include.h"
#include "window_detective/main.h"
#include "WindowManager.h"
#include "MessageHandler.h"
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
using namespace inspector;

WindowManager* WindowManager::Current = NULL;

/*------------------------------------------------------------------+
| Initialize singleton instance and other global objects.           |
+------------------------------------------------------------------*/
void WindowManager::initialize() {
    Window::flashHighlighter.create();;
    if (Current != NULL) delete Current;
    Current = new WindowManager();
}

/*------------------------------------------------------------------+
| Constructor                                                       |
+------------------------------------------------------------------*/
WindowManager::WindowManager() :
    allWindows(), allProcesses() {
}

/*------------------------------------------------------------------+
| Rebuild the window list by enumerating over all child windows     |
| of the desktop window. Also refreshes processes and threads.      |
+------------------------------------------------------------------*/
void WindowManager::refreshAllWindows() {
    WindowList::iterator each;

    // Delete old windows
    foreach (Window* each, allWindows) delete each;
    foreach (Process* each, allProcesses) delete each;
    allWindows.clear();
    allProcesses.clear();

    // Enumerate desktop window and all it's children
    allWindows.append(new Window(GetDesktopWindow()));
    EnumChildWindows(GetDesktopWindow(),
                     WindowManager::enumChildWindows,
                     reinterpret_cast<LPARAM>(this));

    // Update window info (silently)
    foreach (Window* each, allWindows) {
        each->update();
    }

    // Built parent and child links. All parents must be set first so
    // that the findChildren algorithm will work.
    foreach (Window* each, allWindows) {
        each->setParent(findParent(each));
    }
    foreach (Window* each, allWindows) {
        each->setChildren(findChildren(each));
    }

    // Set each window's owner process and add it to the list.
    DWORD threadId, processId = -1;
    Process* process = NULL;
    foreach (Window* each, allWindows) {
        threadId = GetWindowThreadProcessId(each->getHandle(), &processId);
        process = findProcess(processId);
        if (!process) {
            process = new Process(processId);
            allProcesses.append(process);
        }
        process->addWindow(each);
        each->setProcess(process);
        each->setThreadId(threadId);
    }
}

/*------------------------------------------------------------------+
| Creates a new Window object from the given handle, adds it to     |
| the list of all windows and notifies anyone interested.           |
+------------------------------------------------------------------*/
Window* WindowManager::addWindow(HWND handle) {
    // Filter out own windows if necessary
    if (!handle || (!Settings::allowInspectOwnWindows && isOwnWindow(handle)))
        return NULL;

    Window* newWindow = new Window(handle);
    allWindows.append(newWindow);

    // Update window and parent/children
    newWindow->update();
    newWindow->setParent(findParent(newWindow));
    newWindow->setChildren(findChildren(newWindow));

    // Update owner process
    DWORD threadId, processId = -1;
    threadId = GetWindowThreadProcessId(handle, &processId);
    Process* process = findProcess(processId);
    if (!process)
        process = addProcess(processId);
    process->addWindow(newWindow);
    newWindow->setProcess(process);
    newWindow->setThreadId(threadId);

    // Notify anyone interested
    emit windowAdded(newWindow);
    Logger::info(TR("Window %1 created.").arg(newWindow->displayName()));
    return newWindow;
}

/*------------------------------------------------------------------+
| Removes the given Window object from the list (assuming it        |
| already exists) and notifies anyone interested.                   |
+------------------------------------------------------------------*/
void WindowManager::removeWindow(Window* window) {
    // Make sure it exists in the list
    if (!window || !find(window->getHandle())) {
        Logger::warning(TR("Attemped to remove non-existant window: %1")
                        .arg(hexString(window ? (uint)window->getHandle() : 0)));
        return;
    }

    // Emit signal first before we actually remove it
    emit windowRemoved(window);
    allWindows.removeOne(window);
    Logger::info(TR("Window %1 destroyed.").arg(window->displayName()));

    Process* ownerProcess = window->getProcess();
    if (ownerProcess) {
        ownerProcess->removeWindow(window);
        // If last in process, remove process
        if (ownerProcess->numWindows() == 0) {
            removeProcess(ownerProcess);
        }
    }
    else {
        Logger::warning(TR("Could not find process that owns the window %1.")
                        .arg(window->displayName()));
    }

    delete window;
}

void WindowManager::removeWindow(HWND handle) {
    removeWindow(find(handle));
}

/*------------------------------------------------------------------+
| Creates a new Process object from the given id, adds it to the    |
| list of all processes and notifies anyone interested.             |
+------------------------------------------------------------------*/
Process* WindowManager::addProcess(uint processId) {
    Process* process = new Process(processId);
    allProcesses.append(process);

    // Notify anyone interested
    emit processAdded(process);
    return process;
}

/*------------------------------------------------------------------+
| Removes the given Process object from the list (assuming it       |
| already exists) and notifies anyone interested.                   |
+------------------------------------------------------------------*/
void WindowManager::removeProcess(Process* process) {
    // Make sure it exists in the list
    if (!process || !findProcess(process->getId())) {
        Logger::warning(TR("Attemped to remove non-existant process: %1")
                        .arg(String::number(process ? process->getId() : 0)));
        return;
    }

    // Emit signal first before we actually remove it
    emit processRemoved(process);
    allProcesses.removeOne(process);
    delete process;
}

/*------------------------------------------------------------------+
| Returns a window with the given handle, or NULL if none exist.    |
+------------------------------------------------------------------*/
Window* WindowManager::find(HWND handle) {
    if (!handle) return NULL;

    WindowList::const_iterator i;
    for (i = allWindows.constBegin(); i != allWindows.constEnd(); i++) {
        if ((*i)->getHandle() == handle)
            return *i;
    }
    return NULL;
}

/*------------------------------------------------------------------+
| Gets the parent handle for the given window and returns the       |
| window which that handle belongs to. Returns NULL if window       |
| does not have a parent.                                           |
+------------------------------------------------------------------*/
// TODO: What's the difference between 'parent' and 'owner'
Window* WindowManager::findParent(Window* window) {
    HWND parentHandle = GetAncestor(window->getHandle(), GA_PARENT);

    // If a window is top-level, GA_PARENT returns 0 but GA_ROOT
    // returns the desktop window!?!?
    if (!parentHandle)
        parentHandle = GetAncestor(window->getHandle(), GA_ROOT);
    return find(parentHandle);
}

/*------------------------------------------------------------------+
| Searches the list of all windows and returns a list of all        |
| windows whos parent is the given window.                          |
+------------------------------------------------------------------*/
WindowList WindowManager::findChildren(Window* window) {
    WindowList children;
    WindowList::const_iterator i;

    for (i = allWindows.constBegin(); i != allWindows.constEnd(); i++) {
        if ((*i)->getParent() == window)
            children.append(*i);
    }
    return children;
}

/*------------------------------------------------------------------+
| Returns the process with the given PID.                           |
+------------------------------------------------------------------*/
Process* WindowManager::findProcess(uint pid) {
    QList<Process*>::const_iterator i;
    for (i = allProcesses.constBegin(); i != allProcesses.constEnd(); i++) {
        if ((*i)->getId() == pid)
            return *i;
    }
    return NULL;
}

/*------------------------------------------------------------------+
| Finds the window/s that match the given criteria.                 |
+------------------------------------------------------------------*/
WindowList WindowManager::find(const SearchCriteria& criteria) {
    WindowList list;
    WindowList::const_iterator i;

    for (i = allWindows.constBegin(); i != allWindows.constEnd(); i++) {
        if (criteria.matches(*i))
            list.append(*i);
    }
    return list;
}

/*------------------------------------------------------------------+
| Returns the window which represents the desktop and is the        |
| parent of all other windows.                                      |
+------------------------------------------------------------------*/
Window* WindowManager::getDesktopWindow() {
    Window* desktopWindow = find(GetDesktopWindow());
    Q_ASSERT(desktopWindow != NULL);
    return desktopWindow;
}

/*------------------------------------------------------------------+
| Returns the top-most window directly under the given position.    |
| Also updates the window object's position just in case it has     |
| moved and we failed to get notified.                              |
+------------------------------------------------------------------*/
Window* WindowManager::getWindowAt(const QPoint& p) {
    HWND handle = WindowFromPoint(POINTFromQPoint(p));
    if (!handle) return NULL;

    Window* window = find(handle);
    if (!window) return NULL;
    window->updateWindowInfo();
    window->fireUpdateEvent(MinorChange);
    return window;
}

/*------------------------------------------------------------------+
| Returns a list of WindowStyles for the given window from the      |
| style bits in the given DWORD.                                    |
| The list of all known window styles is searched for styles        |
| whos value matches the bits set. Usually, a style will only use   |
| one bit, although some are a combination of other style bits.     |
| In this case, both that style, and any others which match the     |
| individual bits, will be added.                                   |
+------------------------------------------------------------------*/
WindowStyleList WindowManager::parseStyle(Window* window,
                                          DWORD styleBits,
                                          bool isExtended) {
    WindowStyleList list;

    // Check general styles first
    foreach (WindowStyle* style, Resources::generalWindowStyles) {
        uint value = style->getValue();
        if (style->isExtended() == isExtended) {
            if ((value & styleBits) == value) {
                list.append(style);
            }
        }
    }

    // Then check the styles specific to the window's class
    foreach (WindowStyle* style, window->getWindowClass()->getApplicableWindowStyles()) {
        uint value = style->getValue();
        if (style->isExtended() == isExtended) {
            if ((value & styleBits) == value) {
                list.append(style);
            }
        }
    }
    return list;
}

/*------------------------------------------------------------------+
| Returns a DWORD containing the bit set based on the given list.   |
+------------------------------------------------------------------*/
uint WindowManager::styleBits(WindowStyleList stylesList) {
    uint styleBits = 0;

    foreach (WindowStyle* style, stylesList) {
        styleBits |= style->getValue();
    }
    return styleBits;
}

/*------------------------------------------------------------------+
| Returns a list of all valid standard styles for the given window. |
+------------------------------------------------------------------*/
WindowStyleList WindowManager::getValidStandardStylesFor(Window* window) {
    WindowStyleList list;

    foreach (WindowStyle* style, Resources::generalWindowStyles) {
        if (!style->isExtended())
            list.append(style);
    }
    foreach (WindowStyle* style, window->getWindowClass()->getApplicableWindowStyles()) {
        if (!style->isExtended())
            list.append(style);
    }
    return list;
}

/*------------------------------------------------------------------+
| Returns a list of all valid extended styles for the given window. |
+------------------------------------------------------------------*/
WindowStyleList WindowManager::getValidExtendedStylesFor(Window* window) {
    WindowStyleList list;

    foreach (WindowStyle* style, Resources::generalWindowStyles) {
        if (style->isExtended())
            list.append(style);
    }
    foreach (WindowStyle* style, window->getWindowClass()->getApplicableWindowStyles()) {
        if (style->isExtended())
            list.append(style);
    }
    return list;
}

/*------------------------------------------------------------------+
| Returns the style with the given name, NULL if there aren't any.  |
+------------------------------------------------------------------*/
WindowStyle* WindowManager::getStyleNamed(const String& name) {
    foreach (WindowStyle* style, Resources::allWindowStyles) {
        if (style->getName() == name)
            return style;
    }
    return NULL;
}

/*------------------------------------------------------------------+
| Returns true if the given window belongs to Window Detective.     |
+------------------------------------------------------------------*/
bool WindowManager::isOwnWindow(HWND handle) {
    DWORD processId = -1;
    GetWindowThreadProcessId(handle, &processId);
    return processId == GetCurrentProcessId();
}

/*------------------------------------------------------------------+
| The callback function to enumerate all child windows.             |
| The WindowManager object that called EnumChildWindows must be     |
| passed as the second parameter (lParam).                          |
+------------------------------------------------------------------*/
BOOL CALLBACK WindowManager::enumChildWindows(HWND hwnd, LPARAM lParam) {
    WindowManager* manager = reinterpret_cast<WindowManager*>(lParam);

    // Filter out own windows if necessary
    if (Settings::allowInspectOwnWindows || !isOwnWindow(hwnd)) {
        manager->allWindows.append(new Window(hwnd));
    }

    // Return TRUE to continue enumeration, FALSE to stop.
    return TRUE;
}

