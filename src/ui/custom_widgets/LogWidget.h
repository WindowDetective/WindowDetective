/////////////////////////////////////////////////////////////////////
// File: LogWidget.h                                               //
// Date: 10/3/10                                                   //
// Desc: Custom widget for displaying log messages. Each log will  //
//   be displayed in a row with data about it in each column (i.e. //
//   log level and description) and can be colour-coded by level.  //
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

#ifndef LOG_WIDGET_H
#define LOG_WIDGET_H

#include "window_detective/include.h"
#include "window_detective/Logger.h"

class LogWidget : public QTreeWidget, public LogListener {
    Q_OBJECT
private:
    QList<LogLevel> filterLevels;// Only keep these levels

public:
    LogWidget(QWidget *parent = 0);
    ~LogWidget();

    void setFilterLevels(QList<LogLevel> levels) { filterLevels = levels; }
    void logAdded(Log* log);
    void logRemoved(Log* log);
};

#endif   // LOG_WIDGET_H