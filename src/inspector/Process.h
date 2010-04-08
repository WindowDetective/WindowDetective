/////////////////////////////////////////////////////////////////////
// File: Process.h                                                 //
// Date: 5/3/10                                                    //
// Desc: Represents a process. Each process has a number of        //
//   windows it owns. Non-windowed processes are ingored.          //
/////////////////////////////////////////////////////////////////////

#ifndef PROCESS_H
#define PROCESS_H

namespace inspector {

// TODO: Not sure if a Thread class is needed...
class Thread {
private:
    DWORD id;               // Thread ID
    Process* ownerProcess;  // Process it belongs to

public:
    Thread() : id(0) {}
    Thread(DWORD tid);
    ~Thread() {}

    uint getId() { return (uint)id; }
};


class Process {
private:
    DWORD id;               // Process ID
    String name;            // Name of the file used to create this process
    String filePath;        // Full path to the executable
    QIcon icon;             // Icon of the process's exe
    QList<Thread> threads;
    QList<Window*> windows; // List of windows it owns

public:
    Process() : id(0) {}
    Process(DWORD pid);
    ~Process() {}

    uint getId() { return (uint)id; }
    String getName() { return name; }
    String getFilePath() { return filePath; }
    const QIcon& getIcon() { return icon; }
    bool hasWindows();
    QList<Window*> getWindows() { return windows; }
    void addWindow(Window* wnd) { windows.append(wnd); }
};

};   // namespace inspector

#endif   // PROCESS_H