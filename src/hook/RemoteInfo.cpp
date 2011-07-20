/////////////////////////////////////////////////////////////////////
// File: RemoteInfo.cpp                                            //
// Date: 14/6/11                                                   //
// Desc: Functions which are called by a delegate function which   //
//   is injected in the remote process by Window Detective.        //
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

#include "stdafx.h"
#include "Hook.h"


/*------------------------------------------------------------------+
| Gets information on a window class.                               |
+------------------------------------------------------------------*/
DWORD GetWindowClassInfoRemote(LPVOID data, DWORD dataSize) {
    // First, a sanity check
	if (dataSize != sizeof(WindowInfoStruct)) return -1;
	WindowInfoStruct* info = (WindowInfoStruct*)data;
    DWORD returnValue = S_OK;

    // Get class info
    if (!GetClassInfoExW(info->hInst, (LPWSTR)info->className, &info->wndClassInfo)) {
        return GetLastError();
    }

    // Load the Gdi32 library, since it may not be loaded in the remote process
    HMODULE hGdi32 = LoadLibraryW(L"Gdi32");
    if (!hGdi32) return GetLastError();

    GetObjectProc fnGetObject = (GetObjectProc)GetProcAddress(hGdi32, "GetObjectW");
    if (!fnGetObject) {
        returnValue = GetLastError();
        goto cleanup;
    }
    HBRUSH hBrush = info->wndClassInfo.hbrBackground;
    if (hBrush) {  // Check if the class actually has a background brush
        if (!fnGetObject(hBrush, sizeof(LOGBRUSH), (LPVOID)&(info->logBrush))) {
            returnValue = GetLastError();
            goto cleanup;
        }
    }

cleanup:
    // Free the Gdi32 library
    if (!FreeLibrary(hGdi32)) return GetLastError();

    return returnValue;
}

/*------------------------------------------------------------------+
| Gets the item data from a ListView.                               |
+------------------------------------------------------------------*/
DWORD GetListViewItemsRemote(LPVOID data, DWORD dataSize) {
    // First, a sanity check
	if (dataSize != sizeof(ListViewItemsStruct)) return -1;
	ListViewItemsStruct* info = (ListViewItemsStruct*)data;

    int numberLeftToGet = info->totalNumber - info->startIndex;
    info->numberRetrieved = ((numberLeftToGet > MAX_LVITEM_COUNT) ? MAX_LVITEM_COUNT : numberLeftToGet);

    for (unsigned int i = 0; i < info->numberRetrieved; i++) {
        LVITEMW lvItem = { 0 };

        // From MSDN: Applications should not assume that the text will necessarily be placed
        // in the specified buffer. The control may instead change the pszText member
        // of the structure to point to the new text, rather than place it in the buffer.
        // For that reason, we can't point it straight at ListViewItemStruct's one
        const UINT bufferSize = arraysize(info->items[i].text);
        WCHAR* buffer = new WCHAR[bufferSize];

        int itemNumber = info->startIndex + i;
        info->items[i].index = itemNumber;
        lvItem.iItem = itemNumber;
        lvItem.iSubItem = 0;
        lvItem.mask = LVIF_TEXT | LVIF_STATE;  //Indicate what data we want to be returned
        lvItem.stateMask = LVIS_SELECTED;
        lvItem.pszText = buffer;
        lvItem.cchTextMax = bufferSize;

        // The struct will be filled with the requested data
        DWORD returnValue;
        LRESULT result = SendMessageTimeoutW(info->handle, LVM_GETITEMW, 0,
                          (LPARAM)&lvItem, SMTO_ABORTIFHUNG, 100, &returnValue);
        if (result && returnValue == TRUE) {
            wcsncpy_s(info->items[i].text, bufferSize, lvItem.pszText, bufferSize);
            delete[] buffer;
            info->items[i].isSelected = (bool)(lvItem.state & LVIS_SELECTED);
        }
        else {
            delete[] buffer;
            return GetLastError();
        }
    }

    return S_OK;
}

/*------------------------------------------------------------------+
| Gets info from a Date/Time picker control.                        |
+------------------------------------------------------------------*/
DWORD GetDateTimeInfoRemote(LPVOID data, DWORD dataSize) {
    // First, a sanity check
	if (dataSize != sizeof(DateTimeInfoStruct)) return -1;
	DateTimeInfoStruct* info = (DateTimeInfoStruct*)data;

    DWORD returnValue = 0;
    LRESULT result = 0;

    // Get the currently selected date/time
    result = SendMessageTimeoutW(info->handle, DTM_GETSYSTEMTIME, 0,
                      (LPARAM)&info->selectedTime, SMTO_ABORTIFHUNG, 100, &returnValue);
    if (result) {
        info->selectedTimeStatus = returnValue;
    }
    else {
        return GetLastError();
    }

    // Get the min/max range
    SYSTEMTIME times[2];
    ZeroMemory(times, sizeof(times));

    result = SendMessageTimeoutW(info->handle, DTM_GETRANGE, 0,
                      (LPARAM)&times, SMTO_ABORTIFHUNG, 100, &returnValue);
    if (result) {
        info->range = returnValue;
        if ((info->range & GDTR_MIN) == GDTR_MIN) {
            if (memcpy_s(&info->minTime, sizeof(SYSTEMTIME), &times[0], sizeof(SYSTEMTIME)) != 0) {
                return ERROR_INVALID_PARAMETER;
            }
        }
        if ((info->range & GDTR_MAX) == GDTR_MAX) {
            if (memcpy_s(&info->maxTime, sizeof(SYSTEMTIME), &times[1], sizeof(SYSTEMTIME)) != 0) {
                return ERROR_INVALID_PARAMETER;
            }
        }
    }
    else {
        return GetLastError();
    }

    return S_OK;
}
