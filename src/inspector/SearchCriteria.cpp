/////////////////////////////////////////////////////////////////////
// File: SearchCriteria.cpp                                        //
// Date: 31/3/10                                                   //
// Desc: Stores criteria used to find a window.                    //
//   One or more criteria can be added and combined using a        //
//   boolean operator AND or OR. A window is found if it matches   //
//   all or any of the criteria, depending on the boolean operator.//
/////////////////////////////////////////////////////////////////////

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
              if (!isCaseSensitive)
                  return window->getText().compare(textToFind, Qt::CaseInsensitive) == 0;
              else
                  return window->getText() == textToFind;
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