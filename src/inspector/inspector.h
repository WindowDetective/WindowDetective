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
    class WindowMessage;
    class Thread;
    class Process;
};

typedef QList<inspector::Window*> WindowList;
typedef QList<inspector::WindowStyle*> WindowStyleList;
typedef QList<inspector::WindowClassStyle*> WindowClassStyleList;

#include "window_misc.h"
#include "Process.h"
#include "Window.h"

#endif   // INSPECTOR_H