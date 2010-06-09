/////////////////////////////////////////////////////////////////////
// File: SearchCriteria.cpp                                        //
// Date: 31/3/10                                                   //
// Desc: Stores criteria used to find a window.                    //
//   One or more criteria can be added and combined using a        //
//   boolean operator AND or OR. A window is found if it matches   //
//   all or any of the criteria, depending on the boolean operator.//
/////////////////////////////////////////////////////////////////////

#ifndef SEARCH_CRITERIA_H
#define SEARCH_CRITERIA_H

#include "inspector.h"

namespace inspector {

/*------------------------------------------------------------------+
| This object models a single search condition (criteria), where a  |
| particular attribute has some relation to a particular value.     |
| Example:                                                          |
|    field = "text", relation = "begins with", value = "blah"       |
+------------------------------------------------------------------*/
class SearchCriteriaItem {
public:
    //SomeType field
    //SomeType relation
    //SomeType value
};

class SearchCriteria {
private:
    QList<SearchCriteriaItem> criteria;

    /*** TODO *********************************
     This is just for basic search. It should be removed once
     i get SearchCriteriaItem working */
    String textToFind;
    uint handleToFind;
    String classToFind;
    int type;
    bool useRegex;
    bool isCaseSensitive;
    /******************************************/

public:
    SearchCriteria();

    /*** TODO *********************************
     This is just for basic search. It should be removed once
     i get SearchCriteriaItem working */
    SearchCriteria(int type, String text, uint handle,
                   String className, bool useRegex,
                   bool isCaseSensitive);
    /******************************************/

    ~SearchCriteria() {};

    bool matches(Window* window) const;
};

};   // namespace inspector

#endif   // SEARCH_CRITERIA_H