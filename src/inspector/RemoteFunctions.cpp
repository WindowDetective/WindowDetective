/////////////////////////////////////////////////////////////////////
// File: RemoteFunctions.cpp                                       //
// Date: 1/7/10                                                    //
// Desc: Defines functions that are injected into a remote thread  //
//   in a process to run code or collect data that can only be     //
//   obtained from that remote process.                            //
//   Note that all functions here MUST NOT make any calls to code  //
//   in this process.                                              //
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

#define STRICT
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include "RemoteFunctions.h"


#define INJECT_PRIVELIDGE  (PROCESS_CREATE_THREAD |     \
                            PROCESS_QUERY_INFORMATION | \
                            PROCESS_VM_OPERATION |      \
                            PROCESS_VM_READ |           \
                            PROCESS_VM_WRITE)

// Typedefs of function pointers
typedef BOOL (WINAPI *GetClassInfoExProc)(HINSTANCE, LPTSTR, WNDCLASSEX*);

/*------------------------------------------------------------------+
| Injects a thread into the given process.                          |
|                                                                   |
| func     - address of function to inject.                         |
| funcSize - size in bytes of the function.                         |
| data     - address of a user-defined structure to be passed to    |
|            the injected thread.                                   |
| dataSize - size in bytes of the structure.                        |
|                                                                   |
| The user-defined structure is also injected into the target       |
| process' address space. When the thread terminates, the structure |
| is read back from the process.                                    |
+------------------------------------------------------------------*/
DWORD InjectRemoteThread(DWORD processId,
                         LPTHREAD_START_ROUTINE func, DWORD funcSize,
                         LPVOID data, DWORD dataSize) {
    HANDLE hProcess;            // Handle to the remote process
    HANDLE hRemoteThread = 0;   // Handle to the injected thread
    DWORD remoteThreadId = 0;   // ID of the injected thread
    DWORD bytesWritten = 0, bytesRead = 0;
    DWORD exitCode;
    DWORD* remoteAddress;
    void* remoteData = 0;

    if (funcSize == 0 || dataSize == 0)
        return ERROR_INVALID_PARAMETER;

    // Total size of all memory copied into remote process
    const DWORD totalSize = funcSize + dataSize + 3;

    // Open the remote process so we can allocate some memory in it
    hProcess = OpenProcess(INJECT_PRIVELIDGE, FALSE, processId);
    if(hProcess == 0)
        return GetLastError();

    // Allocate enough memory in the remote process's address space to hold the
    // binary image of our injection thread, and a copy of the data structure
    remoteAddress = (DWORD*)VirtualAllocEx(hProcess, 0, totalSize,
                        MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!remoteAddress) {
        CloseHandle(hProcess);
        return GetLastError();
    }

    // Write a copy of our injection thread into the remote process
    if (!WriteProcessMemory(hProcess, remoteAddress, func, funcSize, &bytesWritten)) {
        VirtualFreeEx(hProcess, remoteAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return GetLastError();
    }

    // Write a copy of the data to the remote process.
    // This structure MUST start on a 32bit boundary
    remoteData = (void*)((BYTE*)remoteAddress + ((funcSize + 4) & ~ 3));
    if (!WriteProcessMemory(hProcess, remoteData, data, dataSize, &bytesWritten)) {
        VirtualFreeEx(hProcess, remoteAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return GetLastError();
    }

    // Create the remote thread
    hRemoteThread = CreateRemoteThread(hProcess, NULL, 0,
                        (LPTHREAD_START_ROUTINE)remoteAddress,
                        remoteData, 0, &remoteThreadId);
    if (!hRemoteThread) {
        VirtualFreeEx(hProcess, remoteAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return GetLastError();
    }

    // Wait for the thread to terminate
    WaitForSingleObject(hRemoteThread, INFINITE);

    // Read the user data structure back again
    if(!ReadProcessMemory(hProcess, remoteData, data, dataSize, &bytesRead)) {
        CloseHandle(hRemoteThread);
        VirtualFreeEx(hProcess, remoteAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return GetLastError();
    }

    // TODO: This doesn't seem to return 0. What is the correct value this should return?
    //GetExitCodeThread(hRemoteThread, &exitCode);
    exitCode = ERROR_SUCCESS;

    CloseHandle(hRemoteThread);
    VirtualFreeEx(hProcess, remoteAddress, 0, MEM_RELEASE);
    CloseHandle(hProcess);

    return exitCode;
}


/********************************/
/*** GetWindowClassInfoRemote ***/
/********************************/

#define MAX_WINDOW_CLASS_NAME 128

// Data structure to pass to our remote function
struct InjWindowClassInfo {
    GetClassInfoExProc fnGetClassInfoEx;
    HINSTANCE hInst;
    WCHAR className[MAX_WINDOW_CLASS_NAME];
    WNDCLASSEX wndClassInfo;
};

// Function to inject to remote process.
#pragma check_stack(off)
static DWORD WINAPI GetClassInfoProc(LPVOID* pParam) {
    InjWindowClassInfo* injData = (InjWindowClassInfo*)pParam;

    return injData->fnGetClassInfoEx(injData->hInst,
                    (LPTSTR)injData->className, &injData->wndClassInfo);
}
#pragma check_stack(on)

// Size calculated by looking at assembly, rounded to power-of-two
// TODO: Find a better way of determining code size
#ifdef _DEBUG
  #define GetClassInfoProcSize 64
#else
  // Just use same size here. It's likely that the size will be greatest
  // in debug. Allocating *more* space than needed can't hurt.
  #define GetClassInfoProcSize 64
#endif

DWORD GetWindowClassInfoRemote(WCHAR* className, HWND hwnd, WNDCLASSEX* wndClass) {
    // Make sure we are not injecting into our own process
    if (GetWindowThreadProcessId(hwnd, 0) == GetCurrentThreadId())
        return -1;

    // If the class name can't fit in our injection struct
    if (wcslen(className) > MAX_WINDOW_CLASS_NAME)
        return ERROR_BUFFER_OVERFLOW;

    InjWindowClassInfo injData;
    HMODULE hUser32 = GetModuleHandle(L"User32");
    DWORD processId = -1;
    DWORD returnValue;

    // Calculate how many bytes the injected code takes
    DWORD codeSize = GetClassInfoProcSize;

    // Setup the injection structure
    ZeroMemory(&injData, sizeof(injData));

    // Get pointers to the API calls we will be using in the remote thread
    injData.fnGetClassInfoEx = (GetClassInfoExProc)GetProcAddress(hUser32,
                    IsWindowUnicode(hwnd) ? "GetClassInfoExW" : "GetClassInfoExA");

    // Setup the data the API calls will need
    errno_t result = wcsncpy_s(injData.className, MAX_WINDOW_CLASS_NAME,
                               className, MAX_WINDOW_CLASS_NAME);
    if (result != 0)
        return ERROR_INVALID_PARAMETER;
    injData.hInst = (HINSTANCE)GetWindowLong(hwnd, GWL_HINSTANCE);

    // Find the process ID of the process which created the specified window
    GetWindowThreadProcessId(hwnd, &processId);

    returnValue = InjectRemoteThread(processId,
                                    (LPTHREAD_START_ROUTINE)GetClassInfoProc,
                                     codeSize, &injData, sizeof(injData));
    if (returnValue == ERROR_SUCCESS)
        *wndClass = injData.wndClassInfo;

    return returnValue;
}
