/////////////////////////////////////////////////////////////////////
// File: SmartValue.h                                              //
// Date: 31/5/10                                                   //
// Desc: This class stores a "smart" value, a setting which is     //
//   remembered based on the number of times the value is set. It  //
//   contains a current value, as well as a list of previously set //
//   values. The list contains a fixed number of values and acts   //
//   like a queue. When the current value is changed, it is also   //
//   pushed onto the list and the last one is removed.             //
//   When a value is set, it only becomes the current if all other //
//   remembered values are the same. If all values are different,  //
//   then the old current value is kept.                           //
//   This means that the value will only be remembered if the user //
//   frequently sets it to a specific value. If they only change   //
//   it once, then it will not remember it and will keep the value //
//   it had always been. It also can have a threshold which        //
//   ensures that the values don't have to be all identical but    //
//   can be within the threshold.                                  //
/////////////////////////////////////////////////////////////////////

#ifndef SMART_VALUE_H
#define SMART_VALUE_H

#include "window_detective/include.h"

#define WINDOW_POS_THRESHOLD          100 // pixels
#define DEFAULT_MAX_VALUES            3

template <class T>
class SmartValue {
private:
    String name;
    T currentValue;
    QList<T> previousValues;
    int maxValues;

public:
    /*------------------------------------------------------------------+
    | SmartValue Constructor                                            |
    | Creates an object using the registry key "name". The value will   |
    | remember a maximum of "maxValues" values and have no threshold.   |
    +------------------------------------------------------------------*/
    SmartValue(String name, int maxValues) :
        name(name),
        maxValues(maxValues) {
        read();
    }

    ~SmartValue() {}

    /*------------------------------------------------------------------+
    | Reads the member variables from a data stream. The type (T) must  |
    | have defined the appropriate << and >> operators.                 |
    +------------------------------------------------------------------*/
    void read() {
        QSettings reg(APP_NAME, APP_NAME);

        QByteArray regData = reg.value("smartSettings/"+name, "").toByteArray();
        if (!regData.isEmpty()) {
            QDataStream stream(regData);
            stream >> currentValue;
            stream >> previousValues;
        }
        else {
            currentValue = T();
            previousValues = QList<T>();
        }
    }

    /*------------------------------------------------------------------+
    | Writes the member variables to a data stream. The type (T) must   |
    | have defined the appropriate << and >> operators.                 |
    +------------------------------------------------------------------*/
    void store() {
        QSettings reg(APP_NAME, APP_NAME);
        QByteArray data;
        QDataStream stream(&data, QIODevice::WriteOnly);
        stream << currentValue;
        stream << previousValues;
        reg.setValue("smartSettings/"+name, data);
    }

    void store(T newValue) {
        setValue(newValue);
        store();
    }

    T value() { return currentValue; }

    /*------------------------------------------------------------------+
    | Remembers the "newValue" and sets it as the current one if all    |
    | previous values are the same as it.                               |
    +------------------------------------------------------------------*/
    void setValue(T newValue) {
        // Remember new value
        previousValues.prepend(newValue);
        if (previousValues.size() > maxValues)
            previousValues.removeLast();

        // Check if all previous values are the same
        bool allSame = true;
        for (int i = 1; i < previousValues.size(); i++) {
            if (!compareValues(previousValues[i], newValue)) {
                allSame = false;
                break;
            }
        }

        // Make the current value the new one
        // if all previous values are the same
        if (allSame)
            currentValue = newValue;
    }

    virtual bool compareValues(T value1, T value2) {
        return (value1 == value2);
    }
};


template <class T>
class FuzzySmartValue : public SmartValue<T> {
private:
    T threshold;

public:
    /*------------------------------------------------------------------+
    | FuzzySmartValue Constructor                                       |
    | Creates an object using the registry key "name". The value will   |
    | remember a maximum of "maxValues" values and have the given       |
    | threshold. Note that the type must be a numerical value.          |
    +------------------------------------------------------------------*/
    FuzzySmartValue(String name, T threshold, int maxValues) :
        SmartValue(name, maxValues),
        threshold(threshold) {
    }

    bool compareValues(T value1, T value2) {
        T difference = value1 - value2;
        return (difference > -threshold && difference < threshold);
    }
};


/*------------------------------------------------------------------+
| Helper methods for reading & storing values for various things.   |
+------------------------------------------------------------------*/
template <class T>
static T readSmartValue(String name) {
    return SmartValue<T>(name, DEFAULT_MAX_VALUES).value();
}

template <class T>
static void storeSmartValue(String name, T value) {
    SmartValue<T>(name, DEFAULT_MAX_VALUES).store(value);
}

static void storeWindowPos(String name, int value) {
    FuzzySmartValue<int>(name, WINDOW_POS_THRESHOLD, DEFAULT_MAX_VALUES).store(value);
}

#endif   // SMART_VALUE_H