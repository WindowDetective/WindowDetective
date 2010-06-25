/////////////////////////////////////////////////////////////////////
// File: WindowManager.h                                           //
// Date: 20/2/10                                                   //
// Desc: Maintains a list of all windows and provides              //
//   functionality to search for a window and other things.        //
/////////////////////////////////////////////////////////////////////

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
    QMap<String,WindowClass*> allWindowClasses;
    WindowStyleList allWindowStyles;
    WindowStyleList generalWindowStyles;
    WindowClassStyleList allClassStyles;
    QIcon defaultWindowIcon;

    WindowManager();
    ~WindowManager();

    void refreshAllWindows();
    Window* addWindow(HWND handle);
    void removeWindow(HWND handle);
    void removeWindow(Window* window);
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
    void loadWindowClasses();
    void loadWindowStyles();

    // The callback function to enumerate all child windows
    static BOOL CALLBACK enumChildWindows(HWND hwndChild, LPARAM lParam);
};

};   // namespace inspector

#endif   // WINDOW_MANAGER_H
