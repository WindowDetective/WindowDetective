/////////////////////////////////////////////////////////////////////
// File: SystemInfoViewer.h                                        //
// Date: 1/6/11                                                    //
// Desc: Dialog to list all system information obtained by the     //
//   GetSystemMetrics WinAPI function. The name of each metric is  //
//   the SM_* constant defined in the WinAPI.                      //
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

#ifndef SYSTEM_INFO_VIEWER_H
#define SYSTEM_INFO_VIEWER_H

#include "window_detective/include.h"
#include "forms/ui_SystemInfoViewer.h"

class SystemInfoViewer : public QDialog, private Ui::SystemInfoViewer {
    Q_OBJECT
private:
    QMap<uint,COLORREF> defaultSystemColours;

public:
    SystemInfoViewer(QWidget* parent = 0);

protected:
    void readSmartSettings();
    void writeSmartSettings();
    void populateData();
    void populateGeneralInfo();
    void populateSystemMetrics();
    void populateSystemColours();
    void clearData();
    void chooseColour(int row);
    void showEvent(QShowEvent* e);
    void hideEvent(QHideEvent* e);
public slots:
    void colourCellDoubleClicked(int row, int column);
    void resetSystemColours();
};

QT_END_NAMESPACE

#endif // SYSTEM_INFO_VIEWER_H
