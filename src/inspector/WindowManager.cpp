/////////////////////////////////////////////////////////////////////
// File: WindowManager.cpp                                         //
// Date: 20/2/10                                                   //
// Desc: Maintains a list of all windows and provides              //
//   functionality to search for a window and other things.        //
/////////////////////////////////////////////////////////////////////

#include "WindowManager.h"
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
using namespace inspector;

WindowManager* WindowManager::current = NULL;

/*------------------------------------------------------------------+
 | Initialize singleton instance.                                   |
 +------------------------------------------------------------------*/
void WindowManager::initialize() {
    if (current != NULL) delete current;
    current = new WindowManager();
}

/*------------------------------------------------------------------+
 | Constructor                                                      |
 +------------------------------------------------------------------*/
WindowManager::WindowManager() :
    allWindows(),
    allWindowClasses(),
    allWindowStyles(),
    generalWindowStyles(),
    allClassStyles(),
    messageNames() {
    loadWindowClasses();
    loadWindowStyles();
    loadWindowMessages();

    // Load the icon for windows that don't have an icon.
    defaultWindowIcon = QIcon("data/window_class_icons/generic_window.png");
}

/*------------------------------------------------------------------+
 | Destructor                                                       |
 +------------------------------------------------------------------*/
WindowManager::~WindowManager() {
}

/*------------------------------------------------------------------+
 | Helper function for parsing INI files.                           |
 +------------------------------------------------------------------*/
QStringList parseLine(String line) {
    QStringList strings;
    String str;

    QRegExp rx("\\s*((?:\"[^\"]*\")|(?:\\w|\\d)+)\\s*(,|$)");
    int pos = 0;
    while ((pos = rx.indexIn(line, pos)) != -1) {
        str = rx.cap(1);
        if (str.startsWith('\"') && str.endsWith('\"')) {
            str = str.mid(1, str.size()-2);   // Strip quotes
            str = str.replace("\\n", "\n");   // Replace any \n with newline
        }
        strings << str;
        pos += rx.matchedLength();
    }

    return strings;
}

/*------------------------------------------------------------------+
 | Load the list of known Win32 window classes. These are all the   |
 | basic controls such as Static and Button.                        |
 +------------------------------------------------------------------*/
void WindowManager::loadWindowClasses() {
    QFile file("data/window_classes.ini");
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox mb(QMessageBox::Critical, "Window Detective Error",
                TR("Could not read window class data (window_classes.ini)"));
        mb.exec();
        return;
    }

    QTextStream stream(&file);
    String line;
    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("//"))
            continue;

        QStringList values = parseLine(line);
        String name = values.at(0);
        String displayName = values.at(1);
        allWindowClasses.insert(name, new WindowClass(name, displayName));
    }
}

/*------------------------------------------------------------------+
 | Load the list of window style definitions.                       |
 +------------------------------------------------------------------*/
void WindowManager::loadWindowStyles() {
    QFile file("data/window_styles.ini");
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox mb(QMessageBox::Critical, "Window Detective Error",
                TR("Could not read window style data (window_styles.ini)"));
        mb.exec();
        return;
    }

    QTextStream stream(&file);
    String line;
    QStringList classNames, values;
    bool isExtended = false;
    WindowStyle* newStyle = NULL;

    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("//"))
            continue;

        if (line.startsWith("[class:")) {
            classNames = parseLine(line.mid(7, line.size()-8));
        }
        else if (line == "[standard]") {
            isExtended = false;
        }
        else if (line == "[extended]") {
            isExtended = true;
        }
        else {
            values = parseLine(line);
            if (classNames.first() == "all") {
                newStyle = new WindowStyle(true, isExtended);
                allWindowStyles.append(newStyle);
                generalWindowStyles.append(newStyle);
            }
            else {
                newStyle = new WindowStyle(false, isExtended);
                allWindowStyles.append(newStyle);
                WindowClass* wndClass;

                // Add this style to each class's list applicable styles
                for (int i = 0; i < classNames.size(); i++) {
                    wndClass = allWindowClasses[classNames[i]];
                    wndClass->addApplicableStyle(newStyle);
                }
            }
            newStyle->readFrom(values);
        }
    }
}

/*------------------------------------------------------------------+
 | Load the list of names of each window message.                   |
 +------------------------------------------------------------------*/
void WindowManager::loadWindowMessages() {
    QFile file("data/window_messages.ini");
    if (!file.open(QFile::ReadOnly)) {
        QMessageBox mb(QMessageBox::Critical, "Window Detective Error",
                TR("Could not read window message data (window_messages.ini)"));
        mb.exec();
        return;
    }

    QTextStream stream(&file);
    String line;
    while (!stream.atEnd()) {
        line = stream.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("//"))
            continue;

        bool ok;
        QStringList values = parseLine(line);
        uint id = values.at(0).toUInt(&ok, 0);
        String name = values.at(1);
        messageNames.insert(id, name);
    }
}

/*------------------------------------------------------------------+
 | Rebuild the window list by enumerating over all child windows    |
 | of the desktop window. Also refreshes processes and threads.     |
 +------------------------------------------------------------------*/
void WindowManager::refreshAllWindows() {
    QList<Window*>::iterator each;

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

    // Update window info
    foreach (Window* each, allWindows)
        each->update();

    // Built parent and child links. All parents must be set first so
    // that the findChildren algorithm will work.
    foreach (Window* each, allWindows) {
        each->setParent(findParent(each));
    }
    foreach (Window* each, allWindows) {
        each->setChildren(findChildren(each));
    }

    // Now get all processes. Not all processes will have windows,
    // but we still hang on to them just in case.
    DWORD processIds[1024], bytesNeeded, numProcesses;
    if (!EnumProcesses(processIds, sizeof(processIds), &bytesNeeded)) {
        Logger::osWarning(TR("Could not get processes."));
    }

    numProcesses = bytesNeeded / sizeof(DWORD);
    if (bytesNeeded == sizeof(processIds)) {
        Logger::osWarning(TR("There appears to be 1024 or more running"
                    " processes. Some may not show in the process view"));
    }

    // Enumerate all processes
    DWORD ownPID = GetCurrentProcessId();
    for (uint i = 0; i < numProcesses; i++) {
        if (Settings::allowInspectOwnWindows || processIds[i] != ownPID)
            allProcesses.append(new Process(processIds[i]));
    }

    // Set each window's owner process
    DWORD threadId, processId = -1;
    foreach (Window* each, allWindows) {
        threadId = GetWindowThreadProcessId(each->getHandle(), &processId);
        Process* process = findProcess(processId);
        process->addWindow(each);
        each->setProcess(process);
        each->setThreadId(threadId);
    }
}

/*------------------------------------------------------------------+
 | Returns a window with the given handle, or NULL if none exist.   |
 +------------------------------------------------------------------*/
Window* WindowManager::find(HWND handle) {
    QList<Window*>::const_iterator i;
    for (i = allWindows.constBegin(); i != allWindows.constEnd(); i++) {
        if ((*i)->getHandle() == handle)
            return *i;
    }
    return NULL;
}

/*------------------------------------------------------------------+
 | Gets the parent handle for the given window and returns the      |
 | window which that handle belongs to. Returns NULL if window      |
 | does not have a parent.                                          |
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
 | Searches the list of all windows and returns a list of all       |
 | windows whos parent is the given window.                         |
 +------------------------------------------------------------------*/
QList<Window*> WindowManager::findChildren(Window* window) {
    QList<Window*> children;
    QList<Window*>::const_iterator i;

    for (i = allWindows.constBegin(); i != allWindows.constEnd(); i++) {
        if ((*i)->getParent() == window)
            children.append(*i);
    }
    return children;
}

/*------------------------------------------------------------------+
 | Returns the process with the given PID.                          |
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
 | Finds the window/s that match the given criteria.                |
 +------------------------------------------------------------------*/
QList<Window*> WindowManager::find(const SearchCriteria& criteria) {
    QList<Window*> list;
    QList<Window*>::const_iterator i;

    for (i = allWindows.constBegin(); i != allWindows.constEnd(); i++) {
        if (criteria.matches(*i))
            list.append(*i);
    }
    return list;
}

/*------------------------------------------------------------------+
 | Returns the window which represents the desktop and is the       |
 | parent of all other windows.                                     |
 +------------------------------------------------------------------*/
Window* WindowManager::getDesktopWindow() {
    Window* desktopWindow = find(GetDesktopWindow());
    Q_ASSERT(desktopWindow != NULL);
    return desktopWindow;
}

/*------------------------------------------------------------------+
 | Returns the top-most window directly under the given position.   |
 +------------------------------------------------------------------*/
Window* WindowManager::getWindowAt(const QPoint& p) {
    HWND handle = WindowFromPoint(POINTFromQPoint(p));
    if (!handle) return NULL;
    return find(handle);
}

/*------------------------------------------------------------------+
 | Returns a list of WindowStyles for the given window from the     |
 | style bits in the given DWORD.                                   |
 | The list of all known window styles is searched for styles       |
 | whos value matches the bits set. Usually, a style will only use  |
 | one bit, although some are a combination of other style bits.    |
 | In this case, both that style, and any others which match the    |
 | individual bits, will be added.                                  |
 +------------------------------------------------------------------*/
WindowStyleList WindowManager::parseStyle(Window* window,
                                          DWORD styleBits,
                                          bool isExtended) {
    WindowStyleList list;

    // Check general styles first
    foreach (WindowStyle* style, generalWindowStyles) {
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
 | Returns a DWORD containing the bit set based on the given list.  |
 +------------------------------------------------------------------*/
uint WindowManager::styleBits(WindowStyleList stylesList) {
    uint styleBits = 0;

    foreach (WindowStyle* style, stylesList) {
        styleBits |= style->getValue();
    }
    return styleBits;
}

/*------------------------------------------------------------------+
 | Returns a list of all valid standard styles for the given window.|
 +------------------------------------------------------------------*/
WindowStyleList WindowManager::getValidStandardStylesFor(Window* window) {
    WindowStyleList list;

    foreach (WindowStyle* style, generalWindowStyles) {
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
 | Returns a list of all valid extended styles for the given window.|
 +------------------------------------------------------------------*/
WindowStyleList WindowManager::getValidExtendedStylesFor(Window* window) {
    WindowStyleList list;

    foreach (WindowStyle* style, generalWindowStyles) {
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
 | Returns the style with the given name, NULL if there aren't any. |
 +------------------------------------------------------------------*/
WindowStyle* WindowManager::getStyleNamed(const String& name) {
    foreach (WindowStyle* style, allWindowStyles) {
        if (style->getName() == name)
            return style;
    }
    return NULL;
}

/*------------------------------------------------------------------+
 | The callback function to enumerate all child windows. Used by    |
 | EnumChildWindows.                                                |
 | The WindowManager object that called EnumChildWindows must be    |
 | passed as the second parameter (lParam).                         |
 +------------------------------------------------------------------*/
BOOL CALLBACK WindowManager::enumChildWindows(HWND hwnd, LPARAM lParam) {
    WindowManager* manager = reinterpret_cast<WindowManager*>(lParam);

    DWORD processId = -1;
    GetWindowThreadProcessId(hwnd, &processId);
    bool isOwnProcess = (processId == GetCurrentProcessId());

    // Filter out own windows if necessary
    if (Settings::allowInspectOwnWindows || !isOwnProcess) {
        manager->allWindows.append(new Window(hwnd));
    }

    // Return TRUE to continue enumeration, FALSE to stop.
    return TRUE;
}

