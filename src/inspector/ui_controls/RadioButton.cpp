/////////////////////////////////////////////////////////////////////
// File: RadioButton.cpp                                           //
// Date: 18/1/11                                                   //
// Desc: Object that represents a radio button control             //
//   (Button with BS_RADIOBUTTON or BS_AUTORADIOBUTTON style)      //
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

#include "inspector/inspector.h"
#include "inspector/WindowManager.h"
#include "window_detective/Logger.h"
using namespace inspector;


/*------------------------------------------------------------------+
| Radio button is just a special type of button, so indicate that   |
| in the class name.                                                |
+------------------------------------------------------------------*/
String RadioButton::getClassName() {
    return getWindowClass()->getName()+" (Radio Button)";
}

const QIcon RadioButton::getIcon() {
    return Resources::getWindowClassIcon("RadioButton");
}