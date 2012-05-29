//////////////////////////////////////////////////////////////////////////
// File: MessageStructDefinitions.h                                     //
// Date: 2012-02-29                                                     //
// Desc: Dynamically defines structures for passing data from window    //
//   messages to Window Detective.                                      //
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

#ifndef MESSAGE_STRUCT_DEFINITIONS_H
#define MESSAGE_STRUCT_DEFINITIONS_H

#include "inspector/DynamicData.h"


// One for each message param (wParam and lParam)
struct StructDefinitionPair {
    StructDefinition* first;
    StructDefinition* second;

    StructDefinitionPair(StructDefinition* s1, StructDefinition* s2) :
        first(s1), second(s2) {}
};

void createStructDefinitions(QHash<String,StructDefinitionPair*>& container);


#endif  // MESSAGE_STRUCT_DEFINITIONS_H