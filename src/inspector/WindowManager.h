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

class WindowManager {
private:
    static WindowManager* current;   // Singleton instance
public:
    static void initialize();
    static WindowManager* getCurrent() { return current; }

    QList<Window*> allWindows;
    QList<Process*> allProcesses;
    QMap<String,WindowClass*> allWindowClasses;
    WindowStyleList allWindowStyles;
    WindowStyleList generalWindowStyles;
    WindowClassStyleList allClassStyles;
    QHash<int,String> messageNames;
    QIcon defaultWindowIcon;

    WindowManager();
    ~WindowManager();

    void refreshAllWindows();
    Window* find(HWND handle);
    Window* findParent(Window* window);
    QList<Window*> findChildren(Window* window);
    Process* findProcess(uint pid);
    QList<Window*> find(const SearchCriteria& criteria);
    Window* getDesktopWindow();
    Window* getWindowAt(const QPoint& p);
    WindowStyleList parseStyle(Window* window, DWORD styleBits, bool isExtended);
    uint styleBits(WindowStyleList stylesList);
    WindowStyleList getValidStandardStylesFor(Window*);
    WindowStyleList getValidExtendedStylesFor(Window*);
    WindowStyle* getStyleNamed(const String& name);
private:
    void loadWindowClasses();
    void loadWindowStyles();
    void loadWindowMessages();

    // The callback function to enumerate all child windows
    static BOOL CALLBACK enumChildWindows(HWND hwndChild, LPARAM lParam);
};

};   // namespace inspector

#endif   // WINDOW_MANAGER_H
