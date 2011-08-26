/////////////////////////////////////////////////////////////////////
// File: WindowManager.cpp                                         //
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

#include "window_detective/include.h"
#include "window_detective/main.h"
#include "WindowManager.h"
#include "MessageHandler.h"
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
#include "window_detective/QtHelpers.h"
#include "window_detective/StringFormatter.h"
#include "ui/HighlightWindow.h"
using namespace inspector;

WindowManager* WindowManager::Current = NULL;

/*------------------------------------------------------------------+
| Initialize singleton instance and other global objects.           |
+------------------------------------------------------------------*/
void WindowManager::initialize() {
    Window::flashHighlighter.create();
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

    // Disconnect any message monitors first
    MessageHandler::current()->removeAllListeners();

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

    // Built parent links.
    foreach (Window* each, allWindows) {
        each->setParent(findParent(each));
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
| Creates a new window object from the given handle. The type of    |
| window or control is based on it's class and possibly styles      |
| (e.g. CheckBox  is a Button with a special style)                 |
+------------------------------------------------------------------*/
Window* WindowManager::createWindow(HWND handle) {
    WCHAR charData[128];

    // Since we are dealing with known window classes here, we know that the
    // buffer will be big enough. If it fails, just create a normal window
    if (!GetClassName(handle, charData, 128)) {
        return new Window(handle);
    }

    String className = String::fromWCharArray(charData);
    // TODO: Find a better way of doing this
    if (className == "Button") {
        LONG typeStyle = GetWindowLong(handle, GWL_STYLE) & BS_TYPEMASK;
        switch (typeStyle) {
          case BS_CHECKBOX:
          case BS_AUTOCHECKBOX:
          case BS_3STATE:
          case BS_AUTO3STATE: {
              return new CheckBox(handle);
          }
          case BS_RADIOBUTTON:
          case BS_AUTORADIOBUTTON: {
              return new RadioButton(handle);
          }
          case BS_GROUPBOX: {
              return new GroupBox(handle);
          }
          default: {
              // If none of the above is true, then the control is just a Button
              return new Button(handle);
          }
        }
    }
    else if (className == "Edit") {
        return new Edit(handle);
    }
    else if (className == "ComboBox") {
        return new ComboBox(handle);
    }
    else if (className == "ListBox") {
        return new ListBox(handle);
    }
    else if (className == "SysListView32") {
        return new ListView(handle);
    }
    else if (className == "SysDateTimePick32") {
        return new DateTimePicker(handle);
    }
    else if (className == "SysTabControl32") {
        return new Tab(handle);
    }
    else if (className == "msctls_statusbar32") {
        return new StatusBar(handle);
    }
    else if (className == "msctls_progress32") {
        return new ProgressBar(handle);
    }

    // If none of the above checks are true, then the control is just an ordinary window
    return new Window(handle);
}

/*------------------------------------------------------------------+
| Creates a new Window object from the given handle, adds it to     |
| the list of all windows and notifies anyone interested.           |
+------------------------------------------------------------------*/
Window* WindowManager::addWindow(HWND handle) {
    // Filter out own windows if necessary
    if (!handle || (!Settings::allowInspectOwnWindows && isOwnWindow(handle)))
        return NULL;

    Window* newWindow = createWindow(handle);
    allWindows.append(newWindow);

    // Set parent
    Window* parent = findParent(newWindow);
    newWindow->setParent(parent);
    if (!parent) {  // Unless it is the desktop window, it should have a parent
        Logger::warning("WindowManager::addWindow - Cannot find parent for window: " +
                        newWindow->getDisplayName());
    }

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
    Logger::info(TR("Window %1 created.").arg(newWindow->getDisplayName()));
    return newWindow;
}

/*------------------------------------------------------------------+
| Removes the given Window object from the list (assuming it        |
| already exists) and notifies anyone interested.                   |
+------------------------------------------------------------------*/
void WindowManager::removeWindow(Window* window) {
    // Make sure it exists in the list
    if (!window || !find(window->getHandle())) {
        Logger::warning("Attemped to remove non-existant window: " +
                          hexString(window ? (uint)window->getHandle() : 0));
        return;
    }

    // Notify others that the window is being removed, then remove it.
    // Note: I don't have to worry about removing child windows, as they will each be
    // destroyed individually (See MSDN on DestroyWindow and WM_DESTROY).
    emit windowRemoved(window);
    allWindows.removeOne(window);

    Logger::info(TR("Window %1 destroyed.").arg(window->getDisplayName()));

    Process* ownerProcess = window->getProcess();
    if (ownerProcess) {
        ownerProcess->removeWindow(window);
        // If last in process, remove process
        if (ownerProcess->numWindows() == 0) {
            removeProcess(ownerProcess);
        }
    }
    else {
        Logger::warning("WindowManager::removeWindow - "
                        "Could not find process that owns the window " +
                        window->getDisplayName());
    }
    delete window;
}

void WindowManager::removeWindow(HWND handle) {
    removeWindow(find(handle));
}

/*------------------------------------------------------------------+
| Finds an existing class with the given name, or creates a new one |
| if it isn't in the list.                                          |
+------------------------------------------------------------------*/
WindowClass* WindowManager::getWindowClassNamed(String name) {
    WindowClass* theClass = NULL;
    if (Resources::windowClasses.contains(name)) {
        theClass = Resources::windowClasses[name];
    }
    else {
        theClass = new WindowClass(name);
        Resources::windowClasses.insert(name, theClass);
    }
    return theClass;
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
        Logger::warning("WindowManager::removeProcess - "
                        "Attemped to remove non-existant process: " +
                        String::number(process ? process->getId() : 0));
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
    window->invalidateDimensions();
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
WindowStyleList WindowManager::parseStyle(Window* window, DWORD styleBits, bool isExtended) {
    WindowStyleList list;

    // Check general styles first
    foreach (WindowStyle* style, Resources::generalWindowStyles) {
        uint value = style->getValue();
        if (style->isExtended() == isExtended) {
            if (TEST_BITS(styleBits, value)) {
                list.append(style);
            }
        }
    }

    // Then check the styles specific to the window's class
    foreach (WindowStyle* style, window->getWindowClass()->getApplicableStyles()) {
        uint value = style->getValue();
        if (style->isExtended() == isExtended) {
            if (TEST_BITS(styleBits, value)) {
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
    foreach (WindowStyle* style, window->getWindowClass()->getApplicableStyles()) {
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
    foreach (WindowStyle* style, window->getWindowClass()->getApplicableStyles()) {
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
        manager->allWindows.append(manager->createWindow(hwnd));
    }

    // Return TRUE to continue enumeration, FALSE to stop.
    return TRUE;
}

