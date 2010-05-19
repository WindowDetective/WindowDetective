/////////////////////////////////////////////////////////////////////
// File: Hook.h                                                    //
// Date: 12/4/10                                                   //
// Desc: Provides DLL functions for hooking window messages in     //
//   remote processes and communicating with the Window Detective  //
//   application (the exe).                                        //
/////////////////////////////////////////////////////////////////////

#ifndef WD_HOOK_H
#define WD_HOOK_H

/*  TODO
   Define message data structures in xml file which will be used by Window Detective.
   There will be a class like SelfDefinedStructure which contains the block of data
   and an array of uints specifying the offsets of each variable. Data types can be
   nested and will all be stored in the big block, accessed and written recursively.
   For the DLL, it will also need to know these types so it can pass the data to WD.
   But it can't use the xml or objects, so store the types in one big array of shorts,
   in the shared data section. The start of the array will contain a list of
   (message id, offset) pairs. The offset will be to the second portion of the array
   which contains the SelfDefinedStructures (i.e. size, offsets). For efficiency, if
   both wParam and lParam are not pointers to structs and are just integers (or handles)
   then the offset will be -1 and no SDS data needs to be stored for it.
   When DLL gets a message, it pulls the data out and puts it in a block of data. If a
   struct contains a pointer, recursively fill in the data at that location. Then pass
   that block of data to WD. It will know the SDS type of it and create an SDS from it.
   Window Detective would read the xml, create the SDSs, then fill in the array in the
   DLL with that same data.

   See message_types_example.xml for an example.
*/

enum MessageType {
    MessageCall      = 0x01,
    MessageReturn    = 0x02,
    MessageFromQueue = 0x03,
    ModifyMessage    = 0x10
};

/* Holds info about a message which was sent to a window */
struct MessageEvent {
    UINT type;
    HWND hwnd;
    UINT messageId;
    WPARAM wParam;
    LPARAM lParam;
    LRESULT returnValue;
};

extern "C" {

#ifdef WD_HOOK_EXPORTS
  #define WD_HOOK_API __declspec(dllexport)
#else
  #define WD_HOOK_API __declspec(dllimport)
#endif


#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#define MAX_WINDOWS   128

// Public API functions.
// NOTE: Should only be called from local process
WD_HOOK_API void Initialize(HWND hwnd, DWORD pid);
WD_HOOK_API DWORD InstallHook();
WD_HOOK_API DWORD RemoveHook();
WD_HOOK_API void GetWindowsToMonitor(/*in*/ HWND* handles, /*in_out*/ int* size);
WD_HOOK_API bool AddWindowToMonitor(HWND handle);
WD_HOOK_API bool RemoveWindowToMonitor(HWND handle);

// Private internal functions
void SendCopyData(MessageEvent* messageEvent);
bool IsWDWindow(HWND hwnd);
bool IsModifyMessage(UINT messageId);
bool IsWindowToMonitor(HWND hwnd);
void ResetSharedData();

}

#endif   // WD_HOOK_H