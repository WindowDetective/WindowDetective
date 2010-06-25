/////////////////////////////////////////////////////////////////////
// File: LogWidget.cpp                                             //
// Date: 10/3/10                                                   //
// Desc: Custom widget for displaying log messages. Each log will  //
//   be displayed in a row with data about it in each column (i.e. //
//   log level and description) and can be colour-coded by level.  //
/////////////////////////////////////////////////////////////////////


#include "LogWidget.h"
#include "window_detective/Settings.h"

#define AUTO_SCROLL_PADDING   50

LogWidget::LogWidget(QWidget *parent) :
    QTreeWidget(parent),
    filterLevels() {
    filterLevels << ErrorLevel << WarnLevel << InfoLevel;
    Logger::current()->setListener(this);
}

LogWidget::~LogWidget() {
    Logger::current()->removeListener();
}

void LogWidget::logAdded(Log* log) {
    // "Abuse" the QTreeWidget by only using top-level items to make it
    // look like a list view with columns.
    QTreeWidgetItem* item = new QTreeWidgetItem(this);

    String timeString = log->getTime().toString(Qt::SystemLocaleShortDate);
    item->setText(0, timeString);
    item->setText(1, log->levelName());
    item->setText(2, log->getMessage().simplified());

    // Set background colour based on log level
    QColor backgroundColour;
    switch (log->getLevel()) {
        case ErrorLevel: backgroundColour = QColor(255, 85, 85); break;
        case WarnLevel:  backgroundColour = QColor(255, 170, 85); break;
        case DebugLevel: backgroundColour = QColor(85, 170, 255); break;
        default:         backgroundColour = QColor(255, 255, 255); break;
    }
    item->setBackground(1, QBrush(backgroundColour));

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