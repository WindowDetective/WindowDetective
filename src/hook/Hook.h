/////////////////////////////////////////////////////////////////////
// File: Hook.h                                                    //
// Date: 12/4/10                                                   //
// Desc: Provides DLL functions for hooking window messages in     //
//   remote processes and communicating with the Window Detective  //
//   application (the exe).                                        //
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

#ifndef WD_HOOK_H
#define WD_HOOK_H


extern "C" {

#ifdef WD_HOOK_EXPORTS
  #define WD_HOOK_API __declspec(dllexport)
#else
  #define WD_HOOK_API __declspec(dllimport)
#endif


#define arraysize(a) (sizeof(a)/sizeof(a[0]))

#define MAX_WINDOWS  128


// Typedefs of function pointers
typedef int (WINAPI *GetObjectProc)(HGDIOBJ, int, LPVOID);


/*------------------------------------------------------------------+
| Public functions, structures and enums.                           |
+------------------------------------------------------------------*/
enum MessageType {
    MessageCall      = 0x0001,
    MessageReturn    = 0x0002,
    MessageFromQueue = 0x0003,
    MessageTypeMask  = 0x000F,
    UpdateFlag       = 0x0010
};

// Holds info about a message which was sent to a window
struct MessageEvent {
    UINT type;
    HWND hwnd;
    UINT messageId;
    WPARAM wParam;
    LPARAM lParam;
    LRESULT returnValue;
};


/*------------------------------------------------------------------+
| Local functions.                                                  |
| These should only be called from local process                    |
+------------------------------------------------------------------*/
WD_HOOK_API void  Initialize(HWND hwnd, DWORD pid);
WD_HOOK_API DWORD InstallHook();
WD_HOOK_API DWORD RemoveHook();
WD_HOOK_API void  GetWindowsToMonitor(/*in*/ HWND* handles, /*in_out*/ int* size);
WD_HOOK_API bool  AddWindowToMonitor(HWND handle);
WD_HOOK_API bool  RemoveWindowToMonitor(HWND handle);


/*------------------------------------------------------------------+
| Remote functions.                                                 |
| These are called by a delegate function which is injected in the  |
| remote process by Window Detective.                               |
+------------------------------------------------------------------*/
WD_HOOK_API DWORD GetWindowClassInfoRemote(LPVOID data, DWORD dataSize);
WD_HOOK_API DWORD GetListViewItemsRemote(LPVOID data, DWORD dataSize);


/*------------------------------------------------------------------+
| Structures and data used by remote functions                      |
+------------------------------------------------------------------*/

#define MAX_WINDOW_CLASS_NAME 128

struct WindowInfoStruct {
    /*in*/ HINSTANCE hInst;
    /*in*/ WCHAR className[MAX_WINDOW_CLASS_NAME];
    /*out*/WNDCLASSEXW wndClassInfo;
    /*out*/LOGBRUSH logBrush;
};

#define MAX_LVITEM_COUNT  256

struct ListViewItemStruct {
    //
};

struct ListViewItemsStruct {
    /*in*/ int numberOfItems;     // Can be up to MAX_LVITEM_COUNT
    /*out*/ListViewItemStruct items[MAX_LVITEM_COUNT];
};


/*------------------------------------------------------------------+
| Private functions.                                                |
+------------------------------------------------------------------*/
void SendCopyData(MessageEvent* messageEvent);
bool IsWDWindow(HWND hwnd);
bool IsUpdateMessage(UINT messageId);
bool IsWindowToMonitor(HWND hwnd);
void ResetSharedData();
}

#endif   // WD_HOOK_H