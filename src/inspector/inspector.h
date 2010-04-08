/////////////////////////////////////////////////////////////////////
// File: inspector.h                                               //
// Date: 5/3/10                                                    //
// Desc: Main header file for Win32 window class definitions       //
/////////////////////////////////////////////////////////////////////

#ifndef INSPECTOR_H
#define INSPECTOR_H

#include "window_detective\include.h"

// Forward declarations
namespace inspector {
    class Window;
    class WindowClass;
    class WindowStyle;
    class WindowClassStyle;
    class Thread;
    class Process;
};

#include "window_misc.h"
#include "Process.h"
#include "Window.h"

#endif   // INSPECTOR_H