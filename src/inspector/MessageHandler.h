/////////////////////////////////////////////////////////////////////
// File: MessageHandler.h                                          //
// Date: 21/4/10                                                   //
// Desc: Handles messages from other windows, which are detected   //
//   by the hook DLL.                                              //
/////////////////////////////////////////////////////////////////////

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include "inspector.h"
#include "hook/Hook.h"

namespace inspector {

#define HANDLER_WINDOW_CLASS_NAME  L"MessageHandlerWindow"

/* UI widgets can inherit this to be notified by the MessageHandler */
class WindowMessageListener {
public:
    virtual void messageAdded(WindowMessage* msg) = 0;
    virtual void messageRemoved(WindowMessage* msg) = 0;
    virtual void messageReturned(WindowMessage* msg) = 0;
};

class MessageHandler {
private:
    static MessageHandler* Current;   // Singleton instance
    static HWND hwndReceiver;         // Window to receive messages from DLL
    static bool isWindowClassCreated;
    static void createWindowClass();
    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
    QMap<Window*,WindowMessageListener*> listeners;
public:
    static void initialize();
    static MessageHandler* current() { return Current; }
    QMap<Window*,QList<WindowMessage*>> windowMessages;
    QHash<uint,String> messageNames;

    MessageHandler();
    ~MessageHandler();

    bool installHook();
    bool removeHook();
    void addMessageListener(WindowMessageListener* l, Window* wnd);
    void removeMessageListener(WindowMessageListener* l);
    void messageEvent(const MessageEvent& e);
    void updateEvent(const MessageEvent& e);
    String messageName(uint id);
private:
    void loadWindowMessages();
};

/* C++ Wrapper for Hook DLL */
class HookDll {
public:
    static void initialize(HWND hwnd, DWORD pid) { Initialize(hwnd, pid); }

    static DWORD install() { return InstallHook(); }

    static DWORD remove() { return RemoveHook(); }

    static QList<HWND> getWindowsToMonitor() {
        HWND* handles = new HWND[MAX_WINDOWS];
        int size = MAX_WINDOWS;
        QList<HWND> array;

        GetWindowsToMonitor(handles, &size);
        for (int i = 0; i < size; i++) {
            array.append(handles[i]);
        }

        delete[] handles;
        return array;
    }

    static bool addWindowToMonitor(HWND handle) {
        return AddWindowToMonitor(handle);
    }

    static bool removeWindowToMonitor(HWND handle) {
        return RemoveWindowToMonitor(handle);
    }
};

};   // namespace inspector

#endif   // MESSAGE_HANDLER_H
