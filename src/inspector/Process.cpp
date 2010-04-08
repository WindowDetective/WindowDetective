/////////////////////////////////////////////////////////////////////
// File: Process.cpp                                               //
// Date: 5/3/10                                                    //
// Desc: Represents a process. Each process has a number of        //
//   windows it owns. Non-windowed processes are kept in case they //
//   ever create a window, but they are not shown in the GUI.      //
/////////////////////////////////////////////////////////////////////

#include "inspector.h"
#include "window_detective/Logger.h"
using namespace inspector;


/********************/
/*** Thread class ***/
/********************/

/*-----------------------------------------------------------------+
 | Constructor.                                                    |
 +-----------------------------------------------------------------*/
Thread::Thread(DWORD tid) :
    id(tid) {
}


/*********************/
/*** Process class ***/
/*********************/

/*-----------------------------------------------------------------+
 | Constructor.                                                    |
 +-----------------------------------------------------------------*/
Process::Process(DWORD pid) :
    id(pid), icon() {
    name = TR("<unknown>");
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                  PROCESS_VM_READ, FALSE, id );
    if (hProcess != NULL) {
        HMODULE hModule;
        DWORD bytesNeeded;

        // We only need the first HMODULE enumerated
        if (EnumProcessModules(hProcess, &hModule,
                    sizeof(hModule), &bytesNeeded)) {
            WCHAR* szName = new WCHAR[MAX_PATH];
            WCHAR* szFile = new WCHAR[MAX_PATH];

            // Get process name
            GetModuleBaseName(hProcess, hModule, szName, MAX_PATH);
            name = String::fromWCharArray(szName);

            // Get file path
            GetModuleFileNameEx(hProcess, hModule, szFile, MAX_PATH);
            filePath = String::fromWCharArray(szFile);

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

            delete[] szName;
            delete[] szFile;
        }
    }
    CloseHandle(hProcess);
}

/*-----------------------------------------------------------------+
 | Returns true if this process has created any windows,           |
 | false otherwise. It also returns false if the process is        |
 | unknown, this usually means the user doesn't have access to it. |
 +-----------------------------------------------------------------*/
bool Process::hasWindows() {
    return windows.size() > 0 && name != "<unknown>";
}