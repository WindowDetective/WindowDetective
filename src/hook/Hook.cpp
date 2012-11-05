//////////////////////////////////////////////////////////////////////////
// File: Hook.cpp                                                       //
// Date: 2010-04-12                                                     //
// Desc: Provides DLL functions for hooking window messages in remote   //
//   processes and communicating with the Window Detective application  //
//////////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2012 XTAL256

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

#include "stdafx.h"
#include "Hook.h"

// Uncommenting the following line will make the DLL ignore messages
// from all process except notepad.exe
//#define DEBUG_NOTEPAD_ONLY
bool debugDontMonitor = false;

// Shared data
// Seen by each instance of this DLL mapped into remote processes
// as well as the instance mapped into our exe
#pragma data_seg(".shared")
#pragma comment(linker, "/section:.shared,rws")

 HHOOK callWndHook = NULL;
 HHOOK callWndRetHook = NULL;
 HHOOK getMsgHook = NULL;
 HWND wdHwnd = NULL;                // The window to send WM_COPYDATA messages to
 DWORD wdProcessId = 0;             // Process ID of Window Detective
 HWND windowsToMonitor[MAX_WINDOWS] = { 0 }; // List of windows WD is monitoring
 HWND windowGettingInfo = NULL;     // If WD is getting window info, we don't want to monitor messages from itself.

 // List of all messages which modify the window.
 // These are used to update the window if it has changed
 UINT updateMessages[] = {
     WM_CREATE,      WM_DESTROY,     WM_MOVE,        WM_SIZE,
     WM_SETTEXT,     WM_SHOWWINDOW,  WM_FONTCHANGE,  WM_SETFONT,
     WM_SETICON,     WM_HSCROLL,     WM_VSCROLL,
     WM_WINDOWPOSCHANGING, WM_WINDOWPOSCHANGED,
     WM_STYLECHANGING,     WM_STYLECHANGED, 
 };

#pragma data_seg()
// End of shared data segment


HINSTANCE dllInstance = NULL;
LRESULT CALLBACK CallWndProc(int, WPARAM, LPARAM);
LRESULT CALLBACK CallWndRetProc(int, WPARAM, LPARAM);
LRESULT CALLBACK GetMsgProc(int, WPARAM, LPARAM);

BOOL APIENTRY DllMain(HMODULE module, DWORD reasonForCall, PVOID reserved) {
    // For debugging, ignore messages from all process except notepad.exe
    #if defined _DEBUG && defined DEBUG_NOTEPAD_ONLY
      if (reasonForCall == DLL_PROCESS_ATTACH) {
          char* fullName = new char[MAX_PATH];
          char* baseName;
          GetProcessImageFileNameA(GetCurrentProcess(), fullName, MAX_PATH);
          baseName = strrchr(fullName, '\\');
          baseName = (baseName ? baseName + 1 : fullName);
          if (_stricmp(baseName, "notepad.exe") != 0)
              debugDontMonitor = true;
          delete[] fullName;
      }
    #endif

    dllInstance = (HINSTANCE)module;

    return TRUE;
}

void Initialize(HWND hwnd, DWORD pid) {
    ResetSharedData();  // Just make sure all data is properly initialized
    wdHwnd = hwnd;
    wdProcessId = pid;
}

/*--------------------------------------------------------------------------+
| Installs the hook procedure into the hook chain.                          |
+--------------------------------------------------------------------------*/
DWORD InstallHook() {
    // Hook for calling window procedure
    callWndHook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, dllInstance, 0);
    if (!callWndHook) return GetLastError();

    // Hook for returning from window procedure call
    callWndRetHook = SetWindowsHookEx(WH_CALLWNDPROCRET, CallWndRetProc, dllInstance, 0);
    if (!callWndRetHook) return GetLastError();

    // Hook for getting a message off the queue
    getMsgHook = SetWindowsHookEx(WH_GETMESSAGE, GetMsgProc, dllInstance, 0);
    if (!getMsgHook) return GetLastError();
    
    return S_OK;
}

/*--------------------------------------------------------------------------+
| Removes the hook procedure from the hook chain.                           |
+--------------------------------------------------------------------------*/
DWORD RemoveHook() {
    DWORD result = 0;

    if(!UnhookWindowsHookEx(callWndHook)) result = GetLastError();
    if(!UnhookWindowsHookEx(callWndRetHook)) result = GetLastError();
    if(!UnhookWindowsHookEx(getMsgHook)) result = GetLastError();

    ResetSharedData();
    return result;
}

/*--------------------------------------------------------------------------+
| Hook procedure for GetMessage or PeekMessage functions.                   |
| This function will be called from the remote process.                     |
+--------------------------------------------------------------------------*/
LRESULT CALLBACK GetMsgProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code != HC_ACTION) goto end;

    MSG* msg = (MSG*)lParam;

    // Window Detective will never post messages to a window, so there is no
    // need for the IsGettingInfo check here.
    // TODO: Perhaps also monitor non-removed messages (i.e. peek), and indicate
    //  such information in the UI (different coloured icon perhaps).
    if (!(wParam & PM_REMOVE) || IsWDWindow(msg->hwnd)) {
        goto end;
    }
    bool isUpdate = IsUpdateMessage(msg->message);
    bool isMonitoring = IsWindowToMonitor(msg->hwnd);
    if (isUpdate || isMonitoring) {
        ProcessMessage(msg->hwnd, msg->message, msg->wParam, msg->lParam, 0,
                       (isMonitoring ? MessageFromQueue : 0) | (isUpdate ? UpdateFlag : 0));
    }
    end:
    return CallNextHookEx(getMsgHook, code, wParam, lParam);
}

/*--------------------------------------------------------------------------+
| Hook procedure for messages sent to a window.                             |
| This function will be called from the remote process.                     |
+--------------------------------------------------------------------------*/
LRESULT CALLBACK CallWndProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code != HC_ACTION) goto end;

    CWPSTRUCT* msg = (CWPSTRUCT*)lParam;
    if (IsGettingInfo(msg->hwnd) || IsWDWindow(msg->hwnd)) {
        goto end;
    }
    bool isUpdate = IsUpdateMessage(msg->message);
    bool isMonitoring = IsWindowToMonitor(msg->hwnd);
    if (isUpdate || isMonitoring) {
        ProcessMessage(msg->hwnd, msg->message, msg->wParam, msg->lParam, 0,
                       (isMonitoring ? MessageCall : 0) | (isUpdate ? UpdateFlag : 0));
    }
    end:
    return CallNextHookEx(callWndHook, code, wParam, lParam);
}

/*--------------------------------------------------------------------------+
| Hook procedure for messages processed by a window.                        |
| This function will be called from the remote process.                     |
+--------------------------------------------------------------------------*/
LRESULT CALLBACK CallWndRetProc(int code, WPARAM wParam, LPARAM lParam) {
    if (code != HC_ACTION) goto end;

    CWPRETSTRUCT* msg = (CWPRETSTRUCT*)lParam;
    if (IsGettingInfo(msg->hwnd) || IsWDWindow(msg->hwnd)) {
        goto end;
    }
    if (IsWindowToMonitor(msg->hwnd)) {
        ProcessMessage(msg->hwnd, msg->message, msg->wParam, msg->lParam,
                       msg->lResult, MessageReturn);
    }
    end:
    return CallNextHookEx(callWndRetHook, code, wParam, lParam);
}


/*--------------------------------------------------------------------------+
| Helper functions to copy struct data from message param to our own memory |
+--------------------------------------------------------------------------*/
bool CopyMessageData(MessageEvent& msg, PVOID from, PVOID& to, int size)  {
    if (!from) {
        return true;
    }
    if (!(to = malloc(size))) {
        return false;
    }
    if (memcpy_s(to, size, (const PVOID)from, size) != 0) {
        return false;
    }
    return true;
}
bool CopyMessageData1(MessageEvent& msg, int size) {
    msg.dataSize1 = size;
    return CopyMessageData(msg, (PVOID)msg.wParam, (PVOID)msg.extraData1, size);
}
bool CopyMessageData2(MessageEvent& msg, int size) {
    msg.dataSize2 = size;
    return CopyMessageData(msg, (PVOID)msg.lParam, (PVOID)msg.extraData2, size);
}

/*--------------------------------------------------------------------------+
| Copies the message parameters, and any extra data, into a struct and      |
| sends it to Window Detective.                                             |
| NOTE: The order in which data members are packed into memory must be      |
| identical to how they are read from the SelfDefinedStruct.                |
+--------------------------------------------------------------------------*/
void ProcessMessage(HWND hwnd, UINT msgId, WPARAM wParam, LPARAM lParam,
                    LRESULT returnValue, int type) {
    // Fill the message struct with the basic data
    // A lot of messages just store primitives in the parameters, so there is
    // no other data we need from them.
    MessageEvent msg;
    msg.type = (MessageType)type;
    msg.hwnd = hwnd;
    msg.messageId = msgId;
    msg.wParam = wParam;
    msg.lParam = lParam;
    msg.extraData1 = NULL;
    msg.dataSize1 = 0;
    msg.extraData2 = NULL;
    msg.dataSize2 = 0;
    msg.returnValue = returnValue;

    // Now get any extra data in the message.
    // Some messages have params that point to structs or strings. For these messages
    // we need to copy all that data and pass it along to Window Detective.
    // TODO: This should really check the window class. Some class-specific message ids
    //   are above WM_USER, and hence may also be used by custom window classes. If that
    //   happens, we will be passing a struct back from here but expecting a different
    //   type when receiving it. And that could cause a crash.
    switch (msgId) {
        case WM_SETTEXT: {
            // TODO: Some windows are Unicode, some aren't. I'm not sure how to correctly deal with this
            break;
        }
        case WM_GETMINMAXINFO: {
            if (!CopyMessageData2(msg, sizeof(MINMAXINFO))) goto cleanup;
            break;
        }
        case WM_DRAWITEM: {
            if (!CopyMessageData2(msg, sizeof(DRAWITEMSTRUCT))) goto cleanup;
            break;
        }
        case WM_MEASUREITEM: {
            if (!CopyMessageData2(msg, sizeof(MEASUREITEMSTRUCT))) goto cleanup;
            break;
        }
        case WM_DELETEITEM: {
            if (!CopyMessageData2(msg, sizeof(DELETEITEMSTRUCT))) goto cleanup;
            break;
        }
        case WM_WINDOWPOSCHANGING:
        case WM_WINDOWPOSCHANGED: {
            if (!CopyMessageData2(msg, sizeof(WINDOWPOS))) goto cleanup;
            break;
        }
        case WM_SIZING:
        case WM_MOVING:
        case EM_GETRECT: 
        case EM_SETRECT:
        case LB_GETITEMRECT:
        case CB_GETDROPPEDCONTROLRECT: {
            if (!CopyMessageData2(msg, sizeof(RECT))) goto cleanup;
            break;
        }
        case EM_GETSEL:
        case CB_GETEDITSEL: {
            if (!CopyMessageData1(msg, sizeof(DWORD))) goto cleanup;
            if (!CopyMessageData2(msg, sizeof(DWORD))) goto cleanup;
            break;
        }
    }

    // Send the message, along with any extra data, to the Window Detective process.
    SendCopyData(msg);

    cleanup:   // Extra data is deleted after it has been sent
    if (msg.extraData1) free(msg.extraData1);
    if (msg.extraData2) free(msg.extraData2);
}

/*--------------------------------------------------------------------------+
| Fills the COPYDATASTRUCT with data and sends it to the receiver.          |
+--------------------------------------------------------------------------*/
DWORD SendCopyData(MessageEvent& msg) {
    COPYDATASTRUCT dataStruct;
    DWORD statusCode = S_OK;
    PDWORD_PTR result = 0;

    // Copy message data and extra data into one block of memory
    DWORD totalDataSize = sizeof(MessageEvent) + msg.dataSize1 + msg.dataSize2;
    PVOID totalData = malloc(totalDataSize);
    PVOID destExtra1 = (void*)((char*)totalData + sizeof(MessageEvent));
    PVOID destExtra2 = (void*)((char*)totalData + sizeof(MessageEvent) + msg.dataSize1);
    if (!totalData) {
        statusCode = ERROR_OUTOFMEMORY;
        goto cleanup;
    }
    if (memcpy_s(totalData, sizeof(MessageEvent), &msg, sizeof(MessageEvent)) != 0) {
        statusCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }
    if (memcpy_s(destExtra1, msg.dataSize1, msg.extraData1, msg.dataSize1) != 0) {
        statusCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }
    if (memcpy_s(destExtra2, msg.dataSize2, msg.extraData2, msg.dataSize2) != 0) {
        statusCode = ERROR_INVALID_PARAMETER;
        goto cleanup;
    }

    dataStruct.dwData = 0;
    dataStruct.cbData = totalDataSize;
    dataStruct.lpData = totalData;

    // TODO: Use a better mechanism for sending data, pipes perhaps.
    if (!SendMessageTimeout(wdHwnd, WM_COPYDATA, 0, (LPARAM)&dataStruct,
                            SMTO_ABORTIFHUNG, SEND_COPYDATA_TIMEOUT, result)) {
        statusCode = GetLastError();
        goto cleanup;
    }

    cleanup:
    if (totalData) free(totalData);
    return statusCode;
}

/*--------------------------------------------------------------------------+
| Fills the given buffer with the list of windows to monitor.               |
| Parameters:                                                               |
|   handles (in)  - pointer to a buffer of window handles                   |
|   size (in/out) - size of the buffer. Must be <= MAX_WINDOWS              |
+--------------------------------------------------------------------------*/
void GetWindowsToMonitor(HWND* handles, int* size) {
    if (!size) return;

    int i = 0;
    while (i < *size && i < MAX_WINDOWS && windowsToMonitor[i]) {
        handles[i] = windowsToMonitor[i];
        i++;
    }
    *size = i;
}

/*--------------------------------------------------------------------------+
| Adds the given window handle to the list of windows to monitor.           |
| Returns true if it was successfully added, false if the list is           |
| full and no more can be added.                                            |
+--------------------------------------------------------------------------*/
bool AddWindowToMonitor(HWND handle) {
    int index = 0;
    while (index < MAX_WINDOWS && windowsToMonitor[index]) {
        index++;
    }
    if (index < MAX_WINDOWS) {
        windowsToMonitor[index] = handle;
        return true;
    }
    else {
        return false;
    }
}

/*--------------------------------------------------------------------------+
| Removes the given window handle from the list of windows to               |
| monitor. Returns true if it was successfully removed, false if it         |
| did not exist in the list.                                                |
+--------------------------------------------------------------------------*/
bool RemoveWindowToMonitor(HWND handle) {
    int foundIndex = 0;
    while (foundIndex < MAX_WINDOWS && windowsToMonitor[foundIndex] != handle) {
        foundIndex++;
    }
    int i = foundIndex + 1;
    if (foundIndex >= MAX_WINDOWS)
        return false;             // Could not find window

    while (i < MAX_WINDOWS) {
        windowsToMonitor[i-1] = windowsToMonitor[i];
        i++;
    }
    if (i == MAX_WINDOWS - 1) {
        windowsToMonitor[i] = NULL;
    }
    return true;
}

/*--------------------------------------------------------------------------+
| Removes all window handles from the list of windows to monitor.           |
+--------------------------------------------------------------------------*/
bool RemoveAllWindowsToMonitor() {
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windowsToMonitor[i] = NULL;
    }
    return true;
}

/*--------------------------------------------------------------------------+
| Lets the hook DLL know when Window Detective is about to get information  |
| from the window with the given handle. During this time, any messages     |
| from that window will be ignored, as they will just be from WD itself.    |
+--------------------------------------------------------------------------*/
bool StartGetInfo(HWND handle) {
    if (!windowGettingInfo) {
        windowGettingInfo = handle;
        return true;
    }
    else {
        return false;
    }
}

/*--------------------------------------------------------------------------+
| Lets the hook DLL know that Window Detective has finished getting         |
| information from the given window, and it's messages should continue to   |
| be monitored (if they were before).                                       |
+--------------------------------------------------------------------------*/
bool StopGetInfo(HWND handle) {
    bool isOk = (windowGettingInfo == handle);
    windowGettingInfo = NULL;
    return isOk;
}

bool IsGettingInfo(HWND hwnd) {
    return windowGettingInfo == hwnd;
}

/*--------------------------------------------------------------------------+
| Returns true if the given window belongs to our Window Detective          |
| process, in which case we don't want to monitor it's messages.            |
+--------------------------------------------------------------------------*/
bool IsWDWindow(HWND hwnd) {
    if (debugDontMonitor) return true;
    DWORD windowPID = -1;
    GetWindowThreadProcessId(hwnd, &windowPID);
    return windowPID == wdProcessId;
}

/*--------------------------------------------------------------------------+
| Returns true if the given message id is a message which modifies          |
| the window in any way such that Window Detective needs to update it.      |
+--------------------------------------------------------------------------*/
bool IsUpdateMessage(UINT messageId) {
    for (int i = 0; i < arraysize(updateMessages); i++) {
        if (messageId == updateMessages[i])
            return true;
    }
    return false;
}

/*--------------------------------------------------------------------------+
| Returns true if the given window handle is being monitored by             |
| Window Detective.                                                         |
+--------------------------------------------------------------------------*/
bool IsWindowToMonitor(HWND handle) {
    for (int i = 0; i < MAX_WINDOWS && windowsToMonitor[i]; i++) {
        if (windowsToMonitor[i] == handle)
            return true;
    }
    return false;
}

/*--------------------------------------------------------------------------+
| If the DLL isn't unhooked from a process when Window Detective quits,     |
| then the shared data still exists. And if WD is started again, that       |
| shared data is used instead of being re-initialized. So we need to reset  |
| it here in case that happens.                                             |
+--------------------------------------------------------------------------*/
void ResetSharedData() {
    callWndHook = NULL;
    callWndRetHook = NULL;
    getMsgHook = NULL;
    wdHwnd = NULL;
    wdProcessId = 0;
    for (int i = 0; i < MAX_WINDOWS; i++) {
        windowsToMonitor[i] = NULL;
    }
}
