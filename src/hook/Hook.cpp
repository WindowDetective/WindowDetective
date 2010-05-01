/////////////////////////////////////////////////////////////////////
// File: Hook.cpp                                                  //
// Date: 12/4/10                                                   //
// Desc: Provides DLL functions for hooking window messages in     //
//   remote processes and communicating with the Window Detective  //
//   application (the exe).                                        //
/////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Hook.h"

// Shared data
// Seen by both the instance of this DLL mapped into the remote
// process as well as the instance mapped into our exe
#pragma data_seg(".shared")
#pragma comment(linker, "/section:.shared,rws")

 HHOOK hook = NULL;
 HWND wdHwnd = NULL;    // The window to send WM_COPYDATA messages to
 DWORD wdProcessId = 0; // Process ID of Window Detective
 HWND windowsToMonitor[MAX_WINDOWS] = { 0 };
 bool isMonitoringAll = false;

 // List of all messages which modify the window.
 // These are used to update the window if it has changed
 UINT modifyMessages[] = {
     WM_CREATE,   WM_DESTROY,    WM_MOVE,       WM_SIZE,
     WM_SETTEXT,  WM_SHOWWINDOW, WM_FONTCHANGE, WM_SETFONT,
     WM_WINDOWPOSCHANGING,       WM_SETICON
 };

#pragma data_seg()
// End of shared data segment


HINSTANCE dllInstance = NULL;

BOOL APIENTRY DllMain(HMODULE module, DWORD reasonForCall, LPVOID reserved) {
    // For debugging, only load the DLL in the Notepad.exe process
    // (assuming it is running). Of course, we still want to load it
    // in our own process as well.
    /***  Too hard to do, may not even need it  ***
    #if DEBUG_NOTEPAD_ONLY
        char* name = new char[MAX_PATH];
        GetModuleFileNameEx(GetCurrentProcess(), module, name, MAX_PATH);
        if ((_stricmp(name, "notepad.exe") != 0) &&
            (_stricmp(name, "Window Detective.exe") != 0))
            return FALSE;
        delete[] name;
    #endif*/

    dllInstance = (HINSTANCE)module;

    return TRUE;
}

void Initialize(HWND hwnd, DWORD pid) {
    wdHwnd = hwnd;
    wdProcessId = pid;
}

/*------------------------------------------------------------------+
 | Fills the COPYDATASTRUCT with data and sends it to the receiver. |
 +------------------------------------------------------------------*/
void SendCopyData(MessageEvent* messageEvent, MessageType type) {
    COPYDATASTRUCT dataStruct;
    dataStruct.dwData = type;
    dataStruct.cbData = sizeof(MessageEvent);
    dataStruct.lpData = messageEvent;
    DWORD result;
    // TODO: PostMessage is better as it can prevent deadlocks, BUT it
    // cannot be used for WM_COPYDATA since the data is only valid until
    // the message proc returns.
    SendMessageTimeout(wdHwnd, WM_COPYDATA, 0, (LPARAM)&dataStruct,
            SMTO_ABORTIFHUNG, 10, &result);
}

// TODO: Catch messages here and do stuff with them.
//   Also, use 3 different hooks - WH_CALLWNDPROC and WH_CALLWNDPROCRET
//   for send and return messages, and WH_GETMESSAGE for messages posted
//   on the queue (i think that's what they are for).
/*------------------------------------------------------------------+
 | Hook procedure for GetMessage or PeekMessage functions.          |
 +------------------------------------------------------------------*/
LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam) {
    MSG* msg = (MSG*)lParam;
    if (!IsWDWindow(msg->hwnd)) {
        if (IsModifyMessage(msg->message) || IsWindowToMonitor(msg->hwnd)) {
            MessageEvent messageEvent;
            messageEvent.hwnd = msg->hwnd;
            messageEvent.messageId = msg->message;
            messageEvent.wParam = msg->wParam;
            messageEvent.lParam = msg->lParam;
            SendCopyData(&messageEvent, MessageFromQueue);
        }
    }

    return CallNextHookEx(hook, code, wParam, lParam);
}

/*------------------------------------------------------------------+
 | Hook procedure for messages sent to a window.                    |
 +------------------------------------------------------------------*/
LRESULT CALLBACK CallWndProc(int code, WPARAM wParam, LPARAM lParam) {
    CWPSTRUCT* msg = (CWPSTRUCT*)lParam;
    if (!IsWDWindow(msg->hwnd)) {
        if (IsModifyMessage(msg->message) || IsWindowToMonitor(msg->hwnd)) {
            MessageEvent messageEvent;
            messageEvent.hwnd = msg->hwnd;
            messageEvent.messageId = msg->message;
            messageEvent.wParam = msg->wParam;
            messageEvent.lParam = msg->lParam;
            SendCopyData(&messageEvent, MessageCall);
        }
    }

    return CallNextHookEx(hook, code, wParam, lParam);
}

/*------------------------------------------------------------------+
 | Hook procedure for messages processed by a window.               |
 +------------------------------------------------------------------*/
LRESULT CALLBACK CallWndRetProc(int code, WPARAM wParam, LPARAM lParam) {
    // TODO...
    /*if (!IsWDWindow(msg->hwnd)) {
        if (IsModifyMessage(msg->message) || IsWindowToMonitor(msg->hwnd)) {
        }
    }*/

    return CallNextHookEx(hook, code, wParam, lParam);
}

/*------------------------------------------------------------------+
 | Installs the hook procedure into the hook chain.                 |
 +------------------------------------------------------------------*/
DWORD InstallHook() {
    hook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, dllInstance, 0);
    if (!hook) return GetLastError();
    else return 0;
}

/*------------------------------------------------------------------+
 | Removes the hook procedure from the hook chain.                  |
 +------------------------------------------------------------------*/
DWORD RemoveHook() {
    BOOL result = UnhookWindowsHookEx(hook);
    if (!result) return GetLastError();
    else return 0;
}

/*------------------------------------------------------------------+
 | Returns true if the given window belongs to our Window Detective |
 | process, in which case we don't want to monitor it's messages.   |
 +------------------------------------------------------------------*/
bool IsWDWindow(HWND hwnd) {
    DWORD windowPID = -1;
    GetWindowThreadProcessId(hwnd, &windowPID);
    return windowPID == wdProcessId;
}

/*------------------------------------------------------------------+
 | Returns true if the given message id is a message which modifies |
 | the window in any way such that Window Detective needs to        |
 | update it.                                                       |
 +------------------------------------------------------------------*/
bool IsModifyMessage(UINT messageId) {
    for (int i = 0; i < ARRAY_SIZE(modifyMessages); i++) {
        if (messageId == modifyMessages[i])
            return true;
    }
    return false;
}

/*------------------------------------------------------------------+
 | Returns true if the given window handle is being monitored by    |
 | Window Detective.                                                |
 +------------------------------------------------------------------*/
bool IsWindowToMonitor(HWND handle) {
    if (isMonitoringAll)
        return true;

    // TODO: loop through
    return false;
}