/////////////////////////////////////////////////////////////////////
// File: Error.h                                                   //
// Date: 1/4/10                                                    //
// Desc: Defines base Error class to store an error message and    //
//   the type of error (i.e. the class name).                      //
//   Subclasses can be defined for specific errors.                //
/////////////////////////////////////////////////////////////////////

#ifndef ERROR_H
#define ERROR_H

class Error {
protected:
    String message; // Error message
    String type;    // Class type
public:
    Error(String type = "Error", String msg = "")
        : type(type), message(msg) {}

    inline String getType() const { return type; }
    inline String getMsg() const { return message; }
    inline String getMsgStr() const { return type + ": " + message; }
};

#endif   // ERROR_H