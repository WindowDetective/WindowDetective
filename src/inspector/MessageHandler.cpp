/////////////////////////////////////////////////////////////////////
// File: MessageHandler.h                                          //
// Date: 21/4/10                                                   //
// Desc: Handles messages from other windows, which are detected   //
//   by the hook DLL.                                              //
/////////////////////////////////////////////////////////////////////

#include "MessageHandler.h"
#include "WindowManager.h"
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
using namespace inspector;

MessageHandler* MessageHandler::Current = NULL;
bool MessageHandler::isWindowClassCreated = false;
HWND MessageHandler::hwndReceiver = NULL;

/**********************/
/*** Static methods ***/
/**********************/

/*------------------------------------------------------------------+
 | Creates the window class that any instance of HighlightWindow    |
 | will use. It is called the first time a highlight window is      |
 | created and can only be called once                              |
 +------------------------------------------------------------------*/
void MessageHandler::createWindowClass() {
    if (MessageHandler::isWindowClassCreated)
        return;   // Can only be called once

    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = (WNDPROC)MessageHandler::wndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hIcon = NULL;
    wndclass.hInstance = GetModuleHandle(NULL);
    wndclass.hCursor = NULL;
    wndclass.hbrBackground = NULL;
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = HANDLER_WINDOW_CLASS_NAME;

    if (RegisterClass(&wndclass)) {
        MessageHandler::isWindowClassCreated = true;
    }
    else {
        Logger::osError(TR("Could not register message handler window. "
                    "Window Detective will not be able to monitor "
                    "messages of other windows."));
    }
}

/*------------------------------------------------------------------+
 | Window callback procedure for message handler.                   |
 | Receives WM_COPYDATA messages from the DLL that is injected into |
 | remote processes. The data sent is the window message that the   |
 | remote window has received.                                      |
 +------------------------------------------------------------------*/
LRESULT CALLBACK MessageHandler::wndProc(HWND hwnd, UINT msg,
                    WPARAM wParam, LPARAM lParam) {
    if (msg == WM_COPYDATA) {
        COPYDATASTRUCT* dataStruct = (COPYDATASTRUCT*)lParam;
        MessageEvent* evnt = (MessageEvent*)dataStruct->lpData;
        MessageHandler::current()->messageEvent(*evnt);
        return TRUE;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

/*------------------------------------------------------------------+
 | Initialize singleton instance.                                   |
 +------------------------------------------------------------------*/
void MessageHandler::initialize() {
    if (Current != NULL) delete Current;
    Current = new MessageHandler();
}


/************************/
/*** Instance methods ***/
/************************/

/*------------------------------------------------------------------+
 | Constructor                                                      |
 +------------------------------------------------------------------*/
MessageHandler::MessageHandler() :
    windowMessages() {
    if (!MessageHandler::isWindowClassCreated)
        createWindowClass();

    hwndReceiver = CreateWindowEx(0, HANDLER_WINDOW_CLASS_NAME,
                L"", 0, 0, 0, 0, 0, NULL, NULL,
                GetModuleHandle(NULL), NULL);
    if (!hwndReceiver) {
        Logger::osError(TR("Could not create message handler window. "
                    "Window Detective will not be able to monitor "
                    "messages of other windows."));
    }
    HookDll::initialize(hwndReceiver, GetCurrentProcessId());
    installHook();
}

/*------------------------------------------------------------------+
 | Destructor                                                       |
 +------------------------------------------------------------------*/
MessageHandler::~MessageHandler() {
    removeHook();
    if (!DestroyWindow(hwndReceiver)) {
        Logger::osWarning(TR("Could not destroy message handler window"));
    }
}

/*------------------------------------------------------------------+
 | Installs a global (system-wide) hook to monitor messages being   |
 | sent to and received by windows. The DLL is injected into each   |
 | process that has a message queue.                                |
 +------------------------------------------------------------------*/
bool MessageHandler::installHook() {
    // Call DLL to set hook
    DWORD result = HookDll::install();
    if (!result) {
        Logger::osError(result, TR("Failed to install message hook"));
        return false;
    }

    // Force DLL to inject immediately by sending each window a message
    foreach (Window* each, WindowManager::current()->allWindows) {
        SendMessageTimeout(each->getHandle(), WM_NULL,
                    0, 0, SMTO_ABORTIFHUNG, 10, &result);
    }
    return true;
}

bool MessageHandler::removeHook() {
    // Call DLL to remove hook
    DWORD result = HookDll::remove();
    if (!result) {
        Logger::osError(result, TR("Failed to remove message hook"));
        return false;
    }

    // Force DLL to unmap by sending each window a message
    foreach (Window* each, WindowManager::current()->allWindows) {
        SendMessageTimeout(each->getHandle(), WM_NULL,
                    0, 0, SMTO_ABORTIFHUNG, 10, &result);
    }
    return true;
}

/*------------------------------------------------------------------+
 | Event handler for WM_COPYDATA messages sent from the DLL.        |
 +------------------------------------------------------------------*/
void MessageHandler::messageEvent(const MessageEvent& e) {
    WindowManager* manager = WindowManager::current();
    if (e.messageId == WM_CREATE) {
        manager->addWindow(e.hwnd);
    }
    else if (e.messageId == WM_DESTROY) {
        manager->removeWindow(e.hwnd);
    }
    else {  // changed
        Window* window = manager->find(e.hwnd);
        if (!window) return;
        window->update();
        window->fireUpdateEvent(WindowChanged);

        // Update children if necessary.
        //  TODO: Are there any other messages which need children
        //  updated too? If so, probably refactor to a separate method.
        if (e.messageId == WM_MOVE || e.messageId == WM_SIZE) {
            foreach (Window* child, window->getDescendants()) {
                child->update();
                child->fireUpdateEvent(MinorChange);
            }
        }
    }
}
