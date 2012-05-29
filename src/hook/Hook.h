//////////////////////////////////////////////////////////////////////////
// File: Hook.h                                                         //
// Date: 2010-04-12                                                     //
// Desc: Provides DLL functions for hooking window messages in remote   //
//   processes and communicating with the Window Detective application  //
//////////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2012 XTAL256

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


// Annotations for parameter of functions or members of structs
#define in
#define out
#define in_out

#define arraysize(a) (sizeof(a)/sizeof(a[0]))

#define MAX_WINDOWS  128
#define SEND_COPYDATA_TIMEOUT  100 // ms


/*--------------------------------------------------------------------------+
| Public functions, structures and enums.                                   |
+--------------------------------------------------------------------------*/
enum MessageType {
    MessageCall      = 0x0001,
    MessageReturn    = 0x0002,
    MessageFromQueue = 0x0003,
    MessageTypeMask  = 0x000F,
    UpdateFlag       = 0x0010
};

// Holds info about a message which was sent to a window
struct MessageEvent {
    MessageType type;
    HWND hwnd;
    UINT messageId;
    WPARAM wParam;
    LPARAM lParam;
    PVOID extraData;
    DWORD dataSize;
    LRESULT returnValue;
};


/*--------------------------------------------------------------------------+
| Local functions.                                                          |
| These should only be called from local process                            |
+--------------------------------------------------------------------------*/
WD_HOOK_API void  Initialize(HWND hwnd, DWORD pid);
WD_HOOK_API DWORD InstallHook();
WD_HOOK_API DWORD RemoveHook();
WD_HOOK_API void  GetWindowsToMonitor(in HWND* handles, in_out int* size);
WD_HOOK_API bool  AddWindowToMonitor(HWND handle);
WD_HOOK_API bool  RemoveWindowToMonitor(HWND handle);
WD_HOOK_API bool  RemoveAllWindowsToMonitor();


/*--------------------------------------------------------------------------+
| Remote functions and structures.                                          |
| These are called by a delegate function which is injected in the          |
| remote process by Window Detective.                                       |
+--------------------------------------------------------------------------*/

//-------------------------------------------------------------------
#define MAX_WINDOW_CLASS_NAME 128
struct WindowInfoStruct {
   in   HINSTANCE hInst;
   in   WCHAR className[MAX_WINDOW_CLASS_NAME];
   out  WNDCLASSEXW wndClassInfo;
   out  LOGBRUSH logBrush;
};
WD_HOOK_API DWORD GetWindowClassInfoRemote(PVOID data, DWORD dataSize);


//-------------------------------------------------------------------
#define MAX_LVITEM_COUNT  256
struct ListViewItemStruct {
    UINT index;
    WCHAR text[1024];
    bool isSelected;
};
struct ListViewItemsStruct {
   in   HWND handle;
   in   UINT startIndex;
   in   UINT totalNumber;
   out  ListViewItemStruct items[MAX_LVITEM_COUNT];
   out  UINT numberRetrieved;    // Will be either totalNumber or MAX_LVITEM_COUNT
};
WD_HOOK_API DWORD GetListViewItemsRemote(PVOID data, DWORD dataSize);


//-------------------------------------------------------------------
#define MAX_TABITEM_COUNT  64    // No-one would use more than 64 tab pages, right?
struct TabItemStruct {
    WCHAR text[1024];
    int imageIndex;
    LPARAM lParam;
};
struct TabItemsStruct {
   in   HWND handle;
   in   UINT totalNumber;
   out  TabItemStruct items[MAX_TABITEM_COUNT];
   out  UINT numberRetrieved;    // Will be either totalNumber or MAX_TABITEM_COUNT
};
WD_HOOK_API DWORD GetTabItemsRemote(PVOID data, DWORD dataSize);


//-------------------------------------------------------------------
struct DateTimeInfoStruct {
   in   HWND handle;
   out  SYSTEMTIME selectedTime;
   out  DWORD selectedTimeStatus;
   out  SYSTEMTIME minTime;
   out  SYSTEMTIME maxTime;
   out  DWORD range;
};
WD_HOOK_API DWORD GetDateTimeInfoRemote(PVOID data, DWORD dataSize);


//-------------------------------------------------------------------
#define MAX_SBPART_COUNT  64    // Again, i doubt there would be more than 64 :).
struct StatusBarPartStruct {
    WCHAR text[1024];
    RECT rect;
};
struct StatusBarInfoStruct {
   in   HWND handle;
   in   UINT totalNumber;
   out  StatusBarPartStruct items[MAX_SBPART_COUNT];
   out  UINT numberRetrieved;    // Will be either totalNumber or MAX_SBPART_COUNT
   out  int horzBorder;
   out  int vertBorder;
   out  int padding;
};
WD_HOOK_API DWORD GetStatusBarInfoRemote(PVOID data, DWORD dataSize);


/*--------------------------------------------------------------------------+
| Private functions.                                                        |
+--------------------------------------------------------------------------*/
bool IsWDWindow(HWND hwnd);
bool IsUpdateMessage(UINT messageId);
bool IsWindowToMonitor(HWND hwnd);
void ProcessMessage(HWND hwnd, UINT msgId, WPARAM wParam, LPARAM lParam,
                    LRESULT returnValue, int type);
DWORD SendCopyData(MessageEvent& messageEvent);
void ResetSharedData();
}

#endif   // WD_HOOK_H