/////////////////////////////////////////////////////////////////////
// File: SearchCriteria.cpp                                        //
// Date: 31/3/10                                                   //
// Desc: Stores criteria used to find a window.                    //
//   One or more criteria can be added and combined using a        //
//   boolean operator AND or OR. A window is found if it matches   //
//   all or any of the criteria, depending on the boolean operator.//
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

#include "SearchCriteria.h"
#include "window_detective/Settings.h"
using namespace inspector;

SearchCriteria::SearchCriteria() {
}

/*** TODO *********************************
 This is just for basic search. It should be removed once
 i get SearchCriteriaItem working
*/
SearchCriteria::SearchCriteria(int type, String text, uint handle,
                               String className, bool useRegex,
                               bool isCaseSensitive) :
    type(type),
    textToFind(text),
    handleToFind(handle),
    classToFind(className),
    useRegex(useRegex),
    isCaseSensitive(isCaseSensitive) {
}
/******************************************/

bool SearchCriteria::matches(Window* window) const {
    /*** TODO *********************************
     This is just for basic search. It should be removed once
     i get SearchCriteriaItem working
    */
    switch (type) {
      case 0: {
          if (useRegex) {
              QRegExp regex(textToFind,
                    isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive,
                    Settings::regexType);
              return regex.exactMatch(window->getText());
          }
          else {
              Qt::CaseSensitivity cs = isCaseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
              return window->getText().contains(textToFind, cs);
          }
      }
      case 1: {
          return window->getHandle() == (HWND)handleToFind;
      }
      case 2: {
          return window->getWindowClass()->getName() == classToFind;
      }
    }
    return false;
    /******************************************/
}