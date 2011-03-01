/////////////////////////////////////////////////////////////////////
// File: Logger.cpp                                                //
// Date: 5/3/10                                                    //
// Desc: Provides a mechanism for logging messages and errors.     //
//   Logs can be displayed in the message window of the GUI as     //
//   well as written or streamed to a file.                        //
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

#include "Logger.h"
#include "window_detective/Settings.h"

Logger* Logger::Current = NULL;

/*------------------------------------------------------------------+
| Initialize singleton instance.                                    |
+------------------------------------------------------------------*/
void Logger::initialize() {
    if (Current != NULL) delete Current;
    Current = new Logger();
}


/*****************/
/*** Log class ***/
/*****************/

/*------------------------------------------------------------------+
| Constructor                                                       |
+------------------------------------------------------------------*/
Log::Log(String message, LogLevel level) :
    message(message),
    level(level) {
    timeStamp = QDateTime::currentDateTime();
}

/*------------------------------------------------------------------+
| Copy Constructor                                                  |
+------------------------------------------------------------------*/
Log::Log(const Log& copy) {
    message = copy.message;
    level = copy.level;
    timeStamp = copy.timeStamp;
}

/*------------------------------------------------------------------+
| Returns the string name of the log level.                         |
| Should really be a lookup table or something...                   |
+------------------------------------------------------------------*/
String Log::levelName() {
    String name;
    switch (level) {
        case ErrorLevel: name = "Error"; break;
        case WarnLevel:  name = "Warning"; break;
        case InfoLevel:  name = "Info"; break;
        case DebugLevel: name = "Debug"; break;
        default:         name = "---";
    }
    return name;
}

/*------------------------------------------------------------------+
| Writes this log message and level to the given stream.            |
| Newlines in the message will be stripped and replaced with space. |
+------------------------------------------------------------------*/
void Log::writeTo(QFile* file) {
    QTextStream stream(file);
    stream << timeStamp.toString(Qt::SystemLocaleShortDate) << " - "
           << levelName() << ":\t"
           << message.simplified() << "\n";
}


/********************/
/*** Logger class ***/
/********************/

/*------------------------------------------------------------------+
| Constructor                                                       |
+------------------------------------------------------------------*/
Logger::Logger() :
    logs(),
    maxLogs(1000),
    file(NULL),
    listener(NULL) {
    if (Settings::enableLogging)
        startLoggingToFile();
}

/*------------------------------------------------------------------+
| Destructor                                                        |
+------------------------------------------------------------------*/
Logger::~Logger() {
    if (file)
        delete file;
}

/*------------------------------------------------------------------+
| Appends a new log to the list and writes it to the stream file    |
| if streaming is enabled.                                          |
+------------------------------------------------------------------*/
void Logger::log(String message, LogLevel level) {
    Log *oldestLog, *newLog;

    // Remove any that exceed the limit
    while (logs.size() > maxLogs) {
        oldestLog = logs.takeFirst();
        if (listener) listener->logRemoved(oldestLog);
        delete oldestLog;
    }

    // Add new log
    newLog = new Log(message, level);
    logs.append(newLog);
    if (file) newLog->writeTo(file);
    if (listener) listener->logAdded(newLog);
}

/*------------------------------------------------------------------+
| Logs an error from the Operating System. The error message is     |
| obtained using GetLastError and FormatMessage.                    |
| NOTE: When an error occurs, this function must be called as       |
| soon as possible and before any other OS functions are called.    |
| This is because some functions may reset the last error id.       |
+------------------------------------------------------------------*/
void Logger::logOSMessage(uint errorId, String userMsg, LogLevel level) {
    if (errorId == 0)
        return;        // No error occurred, do nothing

    String osMsg;
    WCHAR* charData = new WCHAR[1024];
    DWORD length = FormatMessage(
                FORMAT_MESSAGE_FROM_SYSTEM |
                FORMAT_MESSAGE_IGNORE_INSERTS,
                NULL, errorId, NULL, charData, 1024, NULL);
    if (length) {
        osMsg = String::fromWCharArray(charData);
    }
    else {
        osMsg = "unknown (" + String::number(errorId) + ")";
    }
    String fullStop = (userMsg.endsWith('.') ? "" : ".");
    log(userMsg + fullStop + "\nOS Error: " + osMsg, level);
    delete charData;
}

void Logger::logOSMessage(String userMsg, LogLevel level) {
    logOSMessage(GetLastError(), userMsg, level);
}

/*------------------------------------------------------------------+
| Sets the output stream on the given file, so that any new logs    |
| are immediately written to the file.                              |
+------------------------------------------------------------------*/
void Logger::setStream(String fileName) {
    stopLoggingToFile();

    file = new QFile(fileName);
    if (!file->open(QIODevice::WriteOnly | QIODevice::Append)) {
        stopLoggingToFile();
    }
}

/*------------------------------------------------------------------+
| Creates a file in the designated output folder (in settings) and  |
| opens it to start logging. The name of the file will be           |
| "Window_Detective_<current date>.log"                             |
+------------------------------------------------------------------*/
void Logger::startLoggingToFile() {
    String dateString = QDate::currentDate().toString(Qt::ISODate);
    String fileName = "Window_Detective_" + dateString + ".log";

    bool hasSlash = Settings::logOutputFolder.endsWith('/');
    setStream(Settings::logOutputFolder + (hasSlash ? "" : "/") + fileName);
}

void Logger::stopLoggingToFile() {
    if (file) {
        delete file;
        file = NULL;
    }
}

/*------------------------------------------------------------------+
| Writes all logs to the given file, overwriting it if it exists.   |
+------------------------------------------------------------------*/
void Logger::saveLogs(String fileName) {
    // TODO
}
