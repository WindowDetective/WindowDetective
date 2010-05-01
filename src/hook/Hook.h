/////////////////////////////////////////////////////////////////////
// File: Hook.h                                                    //
// Date: 12/4/10                                                   //
// Desc: Provides DLL functions for hooking window messages in     //
//   remote processes and communicating with the Window Detective  //
//   application (the exe).                                        //
/////////////////////////////////////////////////////////////////////

#ifndef WD_HOOK_H
#define WD_HOOK_H

/* Holds info about a message which was sent to a window */
struct MessageEvent {
    HWND hwnd;
    UINT messageId;
    WPARAM wParam;
    LPARAM lParam;
    LRESULT returnValue;
};

enum MessageType {
    MessageCall,
    MessageReturn,
    MessageFromQueue
    /* maybe others */
};

extern "C" {

// All files within this DLL are compiled with the WD_HOOK_EXPORTS symbol
// defined on the command line. This symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this
// file see WD_HOOK_API functions as being imported from a DLL, whereas this DLL
// sees symbols defined with this macro as being exported.
#ifdef WD_HOOK_EXPORTS
  #define WD_HOOK_API __declspec(dllexport)
#else
  #define WD_HOOK_API __declspec(dllimport)
#endif

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define MAX_WINDOWS   128

WD_HOOK_API void Initialize(HWND hwnd, DWORD pid);
WD_HOOK_API DWORD InstallHook();
WD_HOOK_API DWORD RemoveHook();

void SendCopyData(MessageEvent* messageEvent, MessageType type);
bool IsWDWindow(HWND hwnd);
bool IsModifyMessage(UINT messageId);
bool IsWindowToMonitor(HWND hwnd);

}

#endif   // WD_HOOK_H