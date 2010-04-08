/////////////////////////////////////////////////////////////////////
// File: Logger.h                                                  //
// Date: 5/3/10                                                    //
// Desc: Provides a mechanism for logging messages and errors.     //
//   Logs can be displayed in the message window of the GUI as     //
//   well as written or streamed to a file.                        //
//   Each log has a severity level, similar to that provided by    //
//   Apache's Log4J library.                                       //
/////////////////////////////////////////////////////////////////////

#ifndef LOGGER_H
#define LOGGER_H

#include "window_detective/include.h"
#include "window_detective/Error.h"

enum LogLevel {
    ErrorLevel,
    WarnLevel,
    InfoLevel,
    DebugLevel,
};

class Log {
private:
    LogLevel level;
    String message;
    QDateTime timeStamp;

public:
    Log(String message, LogLevel level);
    Log(const Log& copy);
    ~Log() {}

    String levelName();
    LogLevel getLevel() { return level; }
    String getMessage() { return message; }
    QDateTime getTimeStamp() { return timeStamp; }
    QDate getDate() { return timeStamp.date(); }
    QTime getTime() { return timeStamp.time(); }
    void writeTo(QFile* file);
};


class Logger : public QObject {
    Q_OBJECT
private:
    static Logger* current;      // Singleton instance
    QList<Log> logs;             // List of all logs
    int maxLogs;                 // Max number of logs to keep
    QFile* file;                 // File to write to, NULL if none
public:
    static void initialize();
    static Logger* getCurrent() { return current; }

    // Static functions for convenience
    static void error(String msg)     { current->logError(msg);    }
    static void warning(String msg)   { current->logWarning(msg);  }
    static void info(String msg)      { current->logInfo(msg);     }
    static void debug(String msg)     { current->logDebug(msg);    }
    static void osWarning(String msg) { current->logOSWarning(msg);}
    static void osError(String msg)   { current->logOSError(msg);  }
    static void error(const Error& e)  { current->logError(e);   }
    static void warning(const Error& e){ current->logWarning(e); }
    static void info(const Error& e)   { current->logInfo(e);    }
    static void debug(const Error& e)  { current->logDebug(e);   }

    Logger();
    ~Logger();

    void logError(String msg)     { log(msg, ErrorLevel); }
    void logWarning(String msg)   { log(msg, WarnLevel);  }
    void logInfo(String msg)      { log(msg, InfoLevel);  }
    void logDebug(String msg)     { log(msg, DebugLevel); }
    void logOSWarning(String msg) { logOSMessage(msg, WarnLevel); }
    void logOSError(String msg)   { logOSMessage(msg, ErrorLevel);}

    void logError(const Error& e)   { logError(e.getMsgStr());   }
    void logWarning(const Error& e) { logWarning(e.getMsgStr()); }
    void logInfo(const Error& e)    { logInfo(e.getMsgStr());    }
    void logDebug(const Error& e)   { logDebug(e.getMsgStr());   }
    void log(const Error& e, LogLevel level) {log(e.getMsgStr(),level);}

    void log(String message, LogLevel level);
    void logOSMessage(String message, LogLevel level);

    void setStream(String fileName);
    void startLoggingToFile();
    void stopLoggingToFile();
    void saveLogs(String fileName);
    int numLogs() { return logs.size(); }
signals:
    void logAdded(Log* log);
};

#endif   // LOGGER_H