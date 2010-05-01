/////////////////////////////////////////////////////////////////////
// File: Process.cpp                                               //
// Date: 5/3/10                                                    //
// Desc: Represents a process. Each process has a number of        //
//   windows it owns. Non-windowed processes are kept in case they //
//   ever create a window, but they are not shown in the GUI.      //
/////////////////////////////////////////////////////////////////////

#include "inspector.h"
#include "window_detective/Logger.h"
#include "window_detective/main.h"
using namespace inspector;


// Function prototypes for DLLs
typedef DWORD (WINAPI *GetModuleFileNameExProc)(HANDLE, HMODULE, LPTSTR, DWORD);
typedef BOOL (WINAPI *QueryFullProcessImageNameProc)(HANDLE, DWORD, LPTSTR, PDWORD);

/*-----------------------------------------------------------------+
 | Constructor.                                                    |
 +-----------------------------------------------------------------*/
Process::Process(DWORD pid) :
    id(pid), icon() {
    name = TR("<unknown>");
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                  PROCESS_VM_READ, FALSE, id );
    // Ignore process 0 and 4 (the system processes)
    if (hProcess != NULL && id != 0 && id != 4) {
        WCHAR* szFile = new WCHAR[MAX_PATH];

        // Get file path and name
        if (moduleFileName(hProcess, szFile, MAX_PATH)) {
            filePath = String::fromWCharArray(szFile);
            int indexOfSlash = filePath.lastIndexOf('\\');
            if (indexOfSlash != -1)
                name = filePath.right(filePath.size()-indexOfSlash-1);

            // Get executable file icon
            SHFILEINFO fileInfo;
            SHGetFileInfo(szFile, 0, &fileInfo, sizeof(fileInfo),
                            SHGFI_ICON | SHGFI_SMALLICON);
            if (fileInfo.hIcon) {
                icon = QIcon(QPixmap::fromWinHICON(fileInfo.hIcon));
                DestroyIcon(fileInfo.hIcon);
            }
            else {
                // TODO: Maybe load generic icon, either from OS or my own
            }
        }
        delete[] szFile;
    }
    CloseHandle(hProcess);
}

// TODO: Think of a better name
/*-----------------------------------------------------------------+
 | Gets the file path of the process and returns it in the szFile  |
 | parameter. On XP systems, the GetModuleFileNameEx function in   |
 | PsApi.dll is used. But Vista and higher can use the             |
 | QueryFullProcessImageName function, which is more robust.       |
 +-----------------------------------------------------------------*/
bool Process::moduleFileName(HANDLE hProcess, WCHAR* szFile, uint size) {
    if (!KernelLibrary || !PsApiLibrary)
        return false;

    DWORD result = 0;
    if (getOSVersion() >= 600) {
        QueryFullProcessImageNameProc fileNameFunction;
        fileNameFunction = (QueryFullProcessImageNameProc)GetProcAddress(
                                    KernelLibrary, "QueryFullProcessImageNameW");
        if (!fileNameFunction)
            return false;
        DWORD sizeIn = size;
        result = (DWORD)fileNameFunction(hProcess, NULL, szFile, (PDWORD)&sizeIn);
    }
    else {
        GetModuleFileNameExProc fileNameFunction;
        fileNameFunction = (GetModuleFileNameExProc)GetProcAddress(
                                    PsApiLibrary, "GetModuleFileNameExW");
        if (!fileNameFunction)
            return false;
        result = (DWORD)fileNameFunction(hProcess, NULL, szFile, (DWORD)size);
    }
    if (!result) {
        Logger::osWarning(TR("Failed to get name of process ") + String::number(id));
        return false;
    }
    return true;
}