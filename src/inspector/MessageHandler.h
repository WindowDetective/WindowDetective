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

class MessageHandler {
private:
    static MessageHandler* Current;   // Singleton instance
    static HWND hwndReceiver;         // Window to receive messages from DLL
    static bool isWindowClassCreated;
    static void createWindowClass();
    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);
public:
    static void initialize();
    static MessageHandler* current() { return Current; }

    QMap<Window*,QList<WindowMessage*>> windowMessages;

    MessageHandler();
    ~MessageHandler();

    bool installHook();
    bool removeHook();
    void messageEvent(const MessageEvent& e);
};

/* C++ Wrapper for Hook DLL */
namespace HookDll {
    inline void initialize(HWND hwnd, DWORD pid) { Initialize(hwnd, pid); }
    inline DWORD install() { return InstallHook(); }
    inline DWORD remove() { return RemoveHook(); }
};

};   // namespace inspector

#endif   // MESSAGE_HANDLER_H
