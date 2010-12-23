/////////////////////////////////////////////////////////////////////
// File: PropertiesWindow.cpp                                      //
// Date: 23/3/10                                                   //
// Desc: Used to display the properties of a window. Typically     //
//   added to an MDI area as a child window.                       //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010 XTAL256

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

#include "PropertiesWindow.h"
#include "StringFormatter.h"
#include "window_detective/main.h"

PropertiesWindow::PropertiesWindow(Window* window, QWidget* parent) :
    QMainWindow(parent),
    client(window) {
    setupUi(this);
    Q_ASSERT(window != NULL);

    connect(actnLocate, SIGNAL(triggered()), this, SLOT(locateActionTriggered()));

    setupProperties();
}

PropertiesWindow::~PropertiesWindow() {
}

/*------------------------------------------------------------------+
| Writes the prop name and value to the stream as a HTML table row. |
+------------------------------------------------------------------*/
template <class T>
void PropertiesWindow::writeProp(QTextStream& stream, String name, T value) {
    StringFormatter<T> formatter(value);
    String row = isEvenRow ? "even" : "odd";
    stream << "<tr class=\"" << row << "\"><td class=\"name\">"
           << name << "</td><td class=\"data\">"
           << formatter.htmlLabel() << "</td></tr>";
    isEvenRow = !isEvenRow;
}

/*------------------------------------------------------------------+
| Builds the HTML for the properties view.                          |
+------------------------------------------------------------------*/
void PropertiesWindow::setupProperties() {
    String htmlString;
    QTextStream stream(&htmlString);

    client->updateExtraInfo();
    WindowClass* windowClass =  client->getWindowClass();
    stream << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "
              "\"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><head>";
    loadCssStyle("PropertiesWindow", stream);
    stream << "</head><body><table width=\"100%\" height=\"100%\" border=\"1\">"
              "<tr><th class=\"main\">Item</th><th class=\"main\">Value</th></tr>";
    isEvenRow = false;
    writeProp<String>(stream, "Text/Title", client->getText());
    writeProp<HWND>(stream, "Handle", client->getHandle());
    if (client->getParent())
        writeProp<HWND>(stream, "Parent Handle", client->getParent()->getHandle());
    writeProp<QRect>(stream, "Dimensions", client->getDimensions());
    writeProp<QPoint>(stream, "Position", client->getPosition());
    writeProp<QSize>(stream, "Size", client->getSize());
    if (client->isChild())
        writeProp<QRect>(stream, "Relative Dimensions", client->getRelativeDimensions());
    writeProp<QRect>(stream, "Client Dimensions", client->getClientDimensions());
    writeProp<String>(stream, "Style bits", hexString(client->getStyleBits()));
    writeProp<WindowStyleList>(stream, "Styles", client->getStandardStyles());
    writeProp<String>(stream, "Extended Style bits", hexString(client->getExStyleBits()));
    writeProp<WindowStyleList>(stream, "Extended Styles", client->getExtendedStyles());
    writeProp<WinFont*>(stream, "Font", client->getFont());
    writeProp<WindowPropList>(stream, "Window Props", client->getProps());
    writeProp<String>(stream, "Owner Process", client->getProcess()->getFilePath());
    writeProp<uint>(stream, "Owner Process ID", client->getProcessId());
    writeProp<uint>(stream, "Owner Thread ID", client->getThreadId());

    stream << "</table></body></br><h2>"
           << TR("Window Class")
           << "</h2><table width=\"100%\" height=\"100%\" border=\"1\">"
              "<tr><th class=\"main\">Item</th><th class=\"main\">Value</th></tr>";
    isEvenRow = false;
    writeProp<String>(stream, "Class Name", windowClass->getDisplayName());
    writeProp<uint>(stream, "Class Extra Bytes", windowClass->getClassExtraBytes());
    writeProp<uint>(stream, "Window Extra Bytes", windowClass->getWindowExtraBytes());
    writeProp<WinBrush*>(stream, "Background Brush", windowClass->getBackgroundBrush());
    stream << "</table></body></html>";

    mainTextEdit->setText(htmlString);
    setWindowTitle(tr("Window Properties - ")+client->getDisplayName());
}

/*------------------------------------------------------------------+
| This just forwards the signal on with this client window.         |
+------------------------------------------------------------------*/
void PropertiesWindow::locateActionTriggered() {
    emit locateWindow(client);
}

/*------------------------------------------------------------------+
| The window has changed, set the properties again.                 |
+------------------------------------------------------------------*/
void PropertiesWindow::update() {
    setupProperties();
}
