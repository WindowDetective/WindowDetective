/////////////////////////////////////////////////////////////////////
// File: LogWidget.h                                               //
// Date: 10/3/10                                                   //
// Desc: Subclass of QTableWidget to provide a custom widget for   //
//   displaying log messages. Each log will be displayed in a row  //
//   with data about it in each column (i.e. log level and         //
//   description) and can be colour-coded by level.                //
/////////////////////////////////////////////////////////////////////

#ifndef LOG_WIDGET_H
#define LOG_WIDGET_H

#include "window_detective/include.h"
#include "window_detective/Logger.h"

class LogWidget : public QTableWidget {
    Q_OBJECT
private:
    QList<LogLevel> filterLevels;// Only keep these levels

public:
    LogWidget(QWidget *parent = 0);
    ~LogWidget() {}

    void setFilterLevels(QList<LogLevel> levels) { filterLevels = levels; }
private slots:
    void addLog(Log* log);
};

#endif   // LOG_WIDGET_H