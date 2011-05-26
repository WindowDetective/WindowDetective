/////////////////////////////////////////////////////////////////////
// File: Process.h                                                 //
// Date: 5/3/10                                                    //
// Desc: Represents a process. Each process has a number of        //
//   windows it owns. Non-windowed processes are ingored.          //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2011 XTAL256

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
    void addWindow(Window* window);
    void removeWindow(Window* window);
private:
    void loadGenericIcon();
    bool moduleFileName(HANDLE hProcess, WCHAR* szFile, uint size);
};

};   // namespace inspector

#endif   // PROCESS_H