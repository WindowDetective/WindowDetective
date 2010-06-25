/////////////////////////////////////////////////////////////////////
// File: Process.h                                                 //
// Date: 5/3/10                                                    //
// Desc: Represents a process. Each process has a number of        //
//   windows it owns. Non-windowed processes are ingored.          //
/////////////////////////////////////////////////////////////////////

#ifndef PROCESS_H
#define PROCESS_H

namespace inspector {

class Process {
private:
    DWORD id;               // Process ID
    String name;            // Name of the file used to create this process
    String filePath;        // Full path to the executable
    QIcon icon;             // Icon of the process's exe
    WindowList windows;     // List of windows it owns

public:
    Process() : id(0) {}
    Process(DWORD pid);
    ~Process() {}

    uint getId() { return (uint)id; }
    String getName() { return name; }
    String getFilePath() { return filePath; }
    const QIcon& getIcon() { return icon; }
    WindowList getWindows() { return windows; }
    int numWindows() { return windows.size(); }
    void addWindow(Window* wnd) { windows.append(wnd); }
    void removeWindow(Window* wnd) { windows.removeOne(wnd); }
private:
    void loadGenericIcon();
    bool moduleFileName(HANDLE hProcess, WCHAR* szFile, uint size);
};

};   // namespace inspector

#endif   // PROCESS_H