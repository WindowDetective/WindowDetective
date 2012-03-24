//////////////////////////////////////////////////////////////////////////
// File: DynamicStruct.cpp                                              //
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

#include "DynamicStruct.h"


//QHash<String,PrimitiveType> StructDefinition::primitiveTypes;
//QHash<String,char*> StructDefinition::typePrintFormats;
//QList<char*> StructDefinition::formatStringCache;


/******************************/
/*** StructDefinition class ***/
/******************************/

/*--------------------------------------------------------------------------+
| Constructor.                                                              |
+--------------------------------------------------------------------------*/
StructDefinition::StructDefinition(String name) :
    name(name), fields(), size(0) {
}

/*--------------------------------------------------------------------------+
| Returns the size of the given primitive type.                             |
+--------------------------------------------------------------------------*/
uint StructDefinition::sizeOfPrimitive(PrimitiveType type) {
    switch (type) {
        case Int8: return sizeof(__int8);
        case Int16: return sizeof(__int16);
        case Int32: return sizeof(__int32);
        case Float32: return sizeof(float);
        case Float64: return sizeof(double);
    }
    return 0;
}

/*--------------------------------------------------------------------------+
| Adds a field with the given name, type and format.                        |
+--------------------------------------------------------------------------*/
void StructDefinition::addField(String name, PrimitiveType type, char* format) {
    FieldDefinition field;
    field.name = name;
    field.type = type;
    field.printFormat = format;
    field.offset = this->size;
    this->fields.append(field);
    this->size += sizeOfPrimitive(type);
}

/*--------------------------------------------------------------------------+
| Returns the field definition with the given name.                         |
| Throws an error if one doesn't exist.                                     |
+--------------------------------------------------------------------------*/
const FieldDefinition& StructDefinition::getField(String name) const {
    QList<FieldDefinition>::const_iterator i;
    for (i = fields.begin(); i != fields.end(); i++) {
        if ((*i).name == name) {
            return *i;
        }
    }
    throw InvalidFieldError(name);
}


/***************************/
/*** DynamicStruct class ***/
/***************************/

DynamicStruct::DynamicStruct(StructDefinition* defn, void* inData, uint inSize) :
    definition(defn),
    data(NULL),
    size(inSize) {
    init(defn, inData, inSize);
}

DynamicStruct::~DynamicStruct() {
    if (data) free(data);
    data = NULL;
}


/*--------------------------------------------------------------------------+
| Initializes the structure with a copy of the given data.                  |
+--------------------------------------------------------------------------*/
void DynamicStruct::init(StructDefinition* defn, void* inData, uint inSize) {
    // Delete any previous data
    if (data) free(data);

    definition = defn;

    // Allocate memory and copy data pointed at by dataPtr
    size = inSize;
    if (inData && size > 0) {
        data = (byte*)malloc(size);
        memcpy_s(data, size, inData, inSize);
    }
}

/*--------------------------------------------------------------------------+
| Returns a formatted string with the value of the field.                   |
+--------------------------------------------------------------------------*/
String DynamicStruct::format(int fieldIndex) const {
    const FieldDefinition& field = definition->getField(fieldIndex);
    String value;

    // Print value based on it's type
    byte* start = data + field.offset;
    char* format = field.printFormat ? field.printFormat : "%d";
    switch (field.type) {
        case Int8: value.sprintf(format, *(__int8*)start); break;
        case Int16: value.sprintf(format, *(__int16*)start); break;
        case Int32: value.sprintf(format, *(__int32*)start); break;
        case Float32: value.sprintf(format, *(float*)start); break;
        case Float64: value.sprintf(format, *(double*)start); break;
    }
    return value;
}

/*--------------------------------------------------------------------------+
| Writes an XML representation of this object to the given stream.          |
+--------------------------------------------------------------------------*/
void DynamicStruct::toXmlStream(QXmlStreamWriter& stream) const {
    const StructDefinition* defn = getDefinition();

    stream.writeStartElement(defn->getName());
    for (int i = 0; i < defn->numFields(); i++) {
        const FieldDefinition& field = defn->getField(i);
        stream.writeTextElement(field.name, format(i));
    }
    stream.writeEndElement();
}