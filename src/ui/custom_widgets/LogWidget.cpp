/////////////////////////////////////////////////////////////////////
// File: LogWidget.cpp                                             //
// Date: 10/3/10                                                   //
// Desc: Subclass of QTableWidget to provide a custom widget for   //
//   displaying log messages. Each log will be displayed in a row  //
//   with data about it in each column (i.e. log level and         //
//   description) and can be colour-coded by level.                //
/////////////////////////////////////////////////////////////////////


#include "LogWidget.h"
#include "window_detective/Settings.h"

#define AUTO_SCROLL_PADDING   50

LogWidget::LogWidget(QWidget *parent) :
    QTableWidget(parent),
    filterLevels() {
    filterLevels << ErrorLevel << WarnLevel << InfoLevel;
    setColumnCount(3);
    QStringList headerLabels;
    headerLabels << "Time" << "Severity" << "Message";
    setHorizontalHeaderLabels(headerLabels);
    Logger::current()->setListener(this);
}

LogWidget::~LogWidget() {
    Logger::current()->removeListener();
}

void LogWidget::logAdded(Log* log) {
    String timeString = log->getTime().toString(Qt::SystemLocaleShortDate);
    QTableWidgetItem* timeItem = new QTableWidgetItem(timeString);
    QTableWidgetItem* levelItem = new QTableWidgetItem(log->levelName());
    QTableWidgetItem* msgItem = new QTableWidgetItem(log->getMessage());

    // Set background colour based on log level
    switch (log->getLevel()) {
        case ErrorLevel: levelItem->setBackground(QBrush(QColor(255, 85, 85))); break;
        case WarnLevel:  levelItem->setBackground(QBrush(QColor(255, 170, 85))); break;
        case DebugLevel: levelItem->setBackground(QBrush(QColor(85, 170, 255))); break;
    }

    insertRow(rowCount());
    setItem(rowCount()-1, 0, timeItem);
    setItem(rowCount()-1, 1, levelItem);
    setItem(rowCount()-1, 2, msgItem);

    // Auto-scroll if necessary
    QScrollBar* sb = verticalScrollBar();
    if (sb && sb->value() >= sb->maximum()-AUTO_SCROLL_PADDING)
        scrollToBottom();
}

void LogWidget::logRemoved(Log* log) {
    // TODO
    //if (first item == log)
    //    remove first
}