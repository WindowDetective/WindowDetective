/////////////////////////////////////////////////////////////////////
// File: inspector.h                                               //
// Date: 5/3/10                                                    //
// Desc: Main header file for Win32 window class definitions       //
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
    class WindowProp;
    class WinBrush;
    class WinFont;
    class Thread;
    class Process;
};

typedef QList<inspector::Window*> WindowList;
typedef QList<inspector::WindowStyle*> WindowStyleList;
typedef QList<inspector::WindowClassStyle*> WindowClassStyleList;
typedef QList<inspector::WindowProp> WindowPropList;

#include "Resources.h"
#include "WindowMisc.h"
#include "Process.h"
#include "Window.h"

#endif   // INSPECTOR_H