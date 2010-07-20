/////////////////////////////////////////////////////////////////////
// File: RemoteFunctions.h                                         //
// Date: 1/7/10                                                    //
// Desc: Defines functions that are injected into a remote thread  //
//   in a process to run code or collect data that can only be     //
//   obtained from that remote process.                            //
//   Note that all functions here MUST NOT make any calls to code  //
//   in this process. For more information, see:                   //
//   http://www.codeproject.com/KB/threads/winspy.aspx#section_3   //
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

#ifndef REMOTE_FUNCTIONS_H
#define REMOTE_FUNCTIONS_H

extern "C" {

// Injects a function into the specified process
DWORD InjectRemoteThread(DWORD processId,
                         LPTHREAD_START_ROUTINE func, DWORD funcSize,
                         LPVOID data, DWORD dataSize);

/* TODO: Some notes:
    * #pragma check_stack supposedly turns off the "stack probe", but that
      is only necessary if you have more than 4k of local vars.
    * Make sure function to inject is static so it's not inrementally linked
    * Define the functions to inject in the cpp not the header file. Do it
      just above the function that uses it (as well as the AfterXXX func)
    * Don't reference any string literals in the injected function.
    * Don't use any switch-case statements (use if-else instead).
    * Make sure /GZ is turned off (in debug). See if there is a #pragma or
      something for it and to control any other debug options.
*/

// These functions setup and inject a function. The actual function which
// they inject is defined in RemoteFunctions.cpp
DWORD GetWindowClassInfoRemote(WCHAR* className, HWND hwnd, WNDCLASSEX* wndClass);
// ... GetXXXRemote();

}

#endif   // REMOTE_FUNCTIONS_H