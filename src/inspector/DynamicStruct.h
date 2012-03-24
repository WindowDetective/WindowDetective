//////////////////////////////////////////////////////////////////////////
// File: DynamicStruct.h                                                //
// Date: 28/7/10                                                        //
// Desc: Classes for defining custom data structures. Raw data is       //
//   stored in a block of memory, where each member can be accessed     //
//   using a defined offset and size. The type is also defined along    //
//   with a format (using printf notation).                             //
//////////////////////////////////////////////////////////////////////////

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

#ifndef DYNAMIC_STRUCT_H
#define DYNAMIC_STRUCT_H

#include "window_detective\include.h"
#include "window_detective\Error.h"


enum PrimitiveType {
    Int8,     // 8 bit byte or character
    Int16,    // 16 bit short integer
    Int32,    // 32 bit integer
    Float32,  // 32 bit floating point number
    Float64,  // 64 bit floating point number (double)
};

struct FieldDefinition {
    String name;
    PrimitiveType type;
    ushort offset;
    char* printFormat;
};


class DynamicStruct;

class StructDefinition {
private:
    //static QHash<String,PrimitiveType> primitiveTypes;    -- Don't need any of this for now
    //static QHash<String,char*> typePrintFormats;
    //static QList<char*> formatStringCache;
public:
    //static void definePrimitive(const QDomElement& node); -- Don't need these either
    //static void defineStruct(const QDomElement& node);
    //static char* addFormatString(const char* str);
    //static char* getTypeFormatString(String typeName);
    static uint sizeOfPrimitive(PrimitiveType type);

private:
    String name;                    // Name of this struct type definition
    QList<FieldDefinition> fields;  // List of fields (members variables)
    uint size;                      // The combined size of the data types

public:
    StructDefinition(String name);
    ~StructDefinition() {}

    void addField(String name, PrimitiveType type, char* format);
    String getName() const { return name; }
    const FieldDefinition& getField(String name) const;
    const FieldDefinition& getField(int i) const { return fields[i]; }
    int numFields() const { return fields.size(); }
    uint sizeInBytes() const { return size; }
};


class DynamicStruct {
private:
    StructDefinition* definition;   // The type of struct (i.e. the 'class' of this 'instance')
    byte* data;                     // The raw data
    uint size;                      // Total size of the data (could be variable)

public:
    DynamicStruct() : definition(NULL), data(NULL), size(0) {}  // Default constructor - NULL data
    DynamicStruct(StructDefinition* defn, void* data, uint size);
    ~DynamicStruct();

    void init(StructDefinition* defn, void* data, uint size);
    const StructDefinition* getDefinition() const { return definition; }
    template <typename T> T get(String fieldName) const;
    String format(int fieldIndex) const;
    uint sizeInBytes() const { return size; }
    bool isNull() const { return data == NULL; }

    void toXmlStream(QXmlStreamWriter& stream) const;
};

/*--------------------------------------------------------------------------+
| Template definition (must be in header).                                  |
| Returns the data at the given field, converted to the given type.         |
+--------------------------------------------------------------------------*/
template <typename T>
T DynamicStruct::get(String fieldName) const {
    // TODO
    // check sizeof(T)
    // return static_cast<T>(offset(fieldName));
    return T();
}


/*********************/
/*** Error classes ***/
/*********************/

class DynamicStructError : public Error {
public:
    DynamicStructError() : Error() {}
    DynamicStructError(String name) : Error(name) {}
};


class InvalidStructError : public DynamicStructError {
public:
    InvalidStructError() : DynamicStructError("Invalid Struct") {}
    InvalidStructError(String name) :
        DynamicStructError("Invalid Struct") {
        message = TR("There is no Dynamic Structure named")
                  + " \"" + name + "\"";
    }
};


class InvalidFieldError : public DynamicStructError {
public:
    InvalidFieldError() : DynamicStructError("Invalid Field") {}
    InvalidFieldError(String name) :
        DynamicStructError("Invalid Field") {
        message = TR("There is no field named")
                  + " \"" + name + "\"";
    }
};


class InvalidTypeError : public DynamicStructError {
public:
    InvalidTypeError() : DynamicStructError("Invalid Type") {}
    InvalidTypeError(String name) :
        DynamicStructError("Invalid Field") {
        message = TR("There is no primitive type named")
                  + " \"" + name + "\"";
    }
};


#endif   // DYNAMIC_STRUCT_H