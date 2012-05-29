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

#include "inspector/MessageStructDefinitions.h"


#define makePair(a, b) (new StructDefinitionPair(a, b))

void createStructDefinitions(QHash<String,StructDefinitionPair*>& container) {
/*    StructDefinition* drawitemstruct = new StructDefinition("DRAWITEMSTRUCT");
    drawitemstruct->addField("CtlType", Int32, "%u");
    drawitemstruct->addField("CtlID", Int32, "%u");
    drawitemstruct->addField("itemID", Int32, "%u");
    drawitemstruct->addField("itemAction", Int32, "%u");
    drawitemstruct->addField("itemState", Int32, "%u");
    drawitemstruct->addField("hwndItem", Int32, "%#08x");
    drawitemstruct->addField("hDC", Int32, "%#08x");
    drawitemstruct->addField("rcItem.left", Int32, "%d");
    drawitemstruct->addField("rcItem.top", Int32, "%d");
    drawitemstruct->addField("rcItem.right", Int32, "%d");
    drawitemstruct->addField("rcItem.bottom", Int32, "%d");
    drawitemstruct->addField("itemData", Int32, "%u");
    container.insert("WM_DRAWITEM",  makePair(NULL, drawitemstruct));

    StructDefinition* measureitemstruct = new StructDefinition("MEASUREITEMSTRUCT");
    measureitemstruct->addField("CtlType", Int32, "%u");
    measureitemstruct->addField("CtlID", Int32, "%u");
    measureitemstruct->addField("itemID", Int32, "%u");
    measureitemstruct->addField("itemWidth", Int32, "%u");
    measureitemstruct->addField("itemHeight", Int32, "%u");
    measureitemstruct->addField("itemData", Int32, "%u");
    container.insert("WM_MEASUREITEM",  makePair(NULL, measureitemstruct));

    StructDefinition* deleteitemstruct = new StructDefinition("DELETEITEMSTRUCT");
    deleteitemstruct->addField("CtlType", Int32, "%u");
    deleteitemstruct->addField("CtlID", Int32, "%u");
    deleteitemstruct->addField("itemID", Int32, "%u");
    deleteitemstruct->addField("hwndItem", Int32, "%#08x");
    deleteitemstruct->addField("itemData", Int32, "%u");
    container.insert("WM_DELETEITEM",  makePair(NULL, deleteitemstruct));

    StructDefinition* windowpos = new StructDefinition("WINDOWPOS");
    windowpos->addField("hwnd", Int32, "%#08x");
    windowpos->addField("hwndInsertAfter", Int32, "%#08x");
    windowpos->addField("x", Int32, "%d");
    windowpos->addField("y", Int32, "%d");
    windowpos->addField("cx", Int32, "%d");
    windowpos->addField("cy", Int32, "%d");
    windowpos->addField("flags", Int32, "%x");
    container.insert("WM_WINDOWPOSCHANGING", makePair(NULL, windowpos));
    container.insert("WM_WINDOWPOSCHANGED",  makePair(NULL, windowpos));
*/
}