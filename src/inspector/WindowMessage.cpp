//////////////////////////////////////////////////////////////////////////
// File: WindowMessage.cpp                                              //
// Date: 2012-04-27                                                     //
// Desc: Classes for window message definitions and instances.          //
//   A message definition stores it's id and name, as well as           //
//   definitions of parameters that are passed in the wParam and lParam //
//   values. A message instance stores the actual data.                 //
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

#include "inspector.h"
#include "WindowManager.hpp"
#include "MessageHandler.h"
#include "window_detective/Logger.h"
#include "inspector/RemoteFunctions.h"
#include "window_detective/StringFormatter.h"
#include "window_detective/QtHelpers.h"


/******************************/
/*** MessageParameter class ***/
/******************************/

/*--------------------------------------------------------------------------+
| Constructor. Creates from the given XML element.                          |
+--------------------------------------------------------------------------*/
MessageParameter::MessageParameter(QDomElement& node) :
    FieldDefinition(node, 0) {

    String varStr = node.attribute("var");

    int index;
    if ((index = varStr.indexOf("LOWORD", 0, Qt::CaseInsensitive)) != -1) {
        part = LoWord;
    }
    else if ((index = varStr.indexOf("HIWORD", 0, Qt::CaseInsensitive)) != -1) {
        part = HiWord;
    }
    else {
        index = 0;
        part = Full;
    }

    if (varStr.indexOf("wParam", index, Qt::CaseInsensitive) != -1) {
        param = WParam;
    }
    else if (varStr.indexOf("lParam", index, Qt::CaseInsensitive) != -1) {
        param = LParam;
    }
    else {
        throw MessageParameterError(TR("Message param \"%1\" should contain "
                                       "wParam or lParam in var attribute").arg(name));
    }
}

/*--------------------------------------------------------------------------+
| Returns a string representation of the given data.                        |
+--------------------------------------------------------------------------*/
String MessageParameter::toString(const WindowMessage* msg) const {
    // Get the data from the parameter
    uint intData = (uint)(param == WParam ? msg->getParam1() : msg->getParam2());

    // Get high or low part if necessary
    if (part == LoWord) {
        intData = LOWORD(intData);
    } else if (part == HiWord) {
        intData = HIWORD(intData);
    }

    // Do the rest of the formatting
    // TODO: This data will always be a certain size (32 or 64 bits). It would be good
    // to let the super method know this, otherwise it would be possible to configure
    // a message to have a param type larger than the actual wParam or lParam values.
    return FieldDefinition::toString((byte*)&intData);
}


/*******************************/
/*** WindowMessageDefn class ***/
/*******************************/

/*--------------------------------------------------------------------------+
| Constructor. Creates from the given XML element.                          |
+--------------------------------------------------------------------------*/
WindowMessageDefn::WindowMessageDefn(QDomElement& node) :
    structDefn(NULL) {
    bool ok;

    id = node.attribute("id").toULong(&ok, 0);
    name = node.attribute("name");

    QDomElement child = node.firstChildElement("param");
    while (!child.isNull()) {
        MessageParameter param(child);
        params.append(param);
        if (param.getType()->isStruct()) {
            if (param.isParam2()) {
                // Remember the struct definition for easy access when creating message instances
                structDefn = static_cast<StructDefinition*>(param.getType());
            }
            else {
                Logger::warning(TR("Structs are not supported for wParam, it will be ignored. "
                                   "See message definition %1.").arg(name));
            }
        }
        child = child.nextSiblingElement("param");
    }
}

/*--------------------------------------------------------------------------+
| Constructor. Creates an application-defined message with the given id.    |
+--------------------------------------------------------------------------*/
WindowMessageDefn::WindowMessageDefn(uint id) :
    id(id) {
    name = nameForId(id);
}

/*--------------------------------------------------------------------------+
| Returns the window class that this message is applicable to, or NULL if   |
| it applies to all (i.e. it's a generic WM_* message).                     |
+--------------------------------------------------------------------------*/
// TODO: Remove if not needed
//WindowClass* WindowMessageDefn::getApplicableClass() {
//}

/*--------------------------------------------------------------------------+
| Returns an appropriate name for an application-defined message id.        |
+--------------------------------------------------------------------------*/
String WindowMessageDefn::nameForId(uint id) {
    if (id >= WM_USER && id <= 0x7FFF) {        // Specific to window class
        return "WM_USER + " + String::number(id-WM_USER);
    }
    else if (id >= WM_APP && id <= 0xBFFF) {    // Application wide
        return "WM_APP + " + String::number(id-WM_APP);
    }
    else if (id >= 0xC000 && id <= 0xFFFF) {    // Registered with a name
        WCHAR szName[256];
        ZeroMemory(szName, 256);
        int length = GetClipboardFormatNameW(id, szName, 256);
        if (length > 0) {
            return String::fromWCharArray(szName, length);
        }
    }
    return TR("Unknown");   // No name matches
}


/***************************/
/*** WindowMessage class ***/
/***************************/

/*--------------------------------------------------------------------------+
| Constructor. Takes id and parameters, no extra data.                      |
+--------------------------------------------------------------------------*/
WindowMessage::WindowMessage(WindowMessageDefn* defn, Window* window,
                             WPARAM wParam, LPARAM lParam) :
    defn(defn), window(window), type((MessageType)0) {
    initParams(wParam, lParam, 0, NULL, 0);
}

/*--------------------------------------------------------------------------+
| Constructs a WindowMessage from a MessageEvent.                           |
+--------------------------------------------------------------------------*/
WindowMessage::WindowMessage(WindowMessageDefn* defn, Window* window, const MessageEvent& evnt) :
    defn(defn),
    window(window),
    type(evnt.type) {
    initParams(evnt.wParam,
               evnt.lParam,
               evnt.returnValue,
               evnt.extraData,
               evnt.dataSize);
}

/*--------------------------------------------------------------------------+
| Initialize parameter data, called by the constructors.                    |
+--------------------------------------------------------------------------*/
void WindowMessage::initParams(WPARAM wParam, LPARAM lParam, LRESULT returnValue,
                               void* extraData, uint dataSize) {
    this->param1 = wParam;
    this->param2 = lParam;
    this->returnValue = returnValue;

    // To make things simpler, we only handle lParam, since historically it is
    // the parameter in which pointers are passed.
    if (defn && extraData && (dataSize > 0)) {
        StructDefinition* structDefn = defn->getStructDefn();
        if (structDefn) {
            this->extraData.init(structDefn, extraData, dataSize);
        }
    }
}

LRESULT WindowMessage::send() {
    if (!window) return 0;

    returnValue = window->sendMessage<LRESULT,WPARAM,LPARAM>(getId(), param1, param2);
    return returnValue;
}

/*--------------------------------------------------------------------------+
| Writes an XML representation of this object to the given stream.          |
+--------------------------------------------------------------------------*/
void WindowMessage::toXmlStream(QXmlStreamWriter& stream) const {
    stream.writeStartElement("windowMessage");
    stream.writeAttribute("id", stringLabel(getId()));
    stream.writeAttribute("name", stringLabel(getName()));
    if (isSent())        { stream.writeAttribute("type", "sent");    }
    else if (isPosted()) { stream.writeAttribute("type", "posted");  }
    else if (isReturn()) { stream.writeAttribute("type", "returned");}

     const WindowMessageDefn* defn = getDefinition();
     const QList<MessageParameter> params = defn->getParams();
     QList<MessageParameter>::const_iterator i;
     for (i = params.begin(); i != params.end(); ++i) {
         if (i->getType()->isStruct()) {
             stream.writeStartElement(i->getName());
             getExtraData().toXmlStream(stream);
             stream.writeEndElement();
         }
         else {
             stream.writeTextElement(i->getName(), i->toString(this));
         }
     }

     if (isReturn()) { stream.writeTextElement("returnValue", hexString((uint)returnValue)); }
    stream.writeEndElement();
}
