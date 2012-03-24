/////////////////////////////////////////////////////////////////////
// File: InfoWindow.hpp                                            //
// Date: 2/3/10                                                    //
// Desc: This window is shown in the top left corner of a window   //
//   which is being highlighted. It's purpose is to display quick  //
//   info on the window such as it's title, window class and       //
//   position.                                                     //
//   Note: Throughout this class, the term 'client' is used to     //
//   describe the window that this will display info for.          //
/////////////////////////////////////////////////////////////////////

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

#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include "window_detective/include.h"
#include "inspector/inspector.h"


class InfoWindow : public QLabel {
    Q_OBJECT
private:
    static QMap<String,String> infoLabelMap; // Maps info labels variable names to display names
    Window* client;

public:
    static void buildInfoLabels();

    InfoWindow(QWidget* parent = 0);
    ~InfoWindow() {}

    void moveTo(Window* window);
    void show();
    void hide();
private:
    void setInfo();
    QRect calcBestDimensions();
};

#endif   // INFO_WINDOW_H