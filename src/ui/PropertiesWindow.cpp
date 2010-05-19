/////////////////////////////////////////////////////////////////////
// File: PropertiesWindow.cpp                                      //
// Date: 23/3/10                                                   //
// Desc: Used to display the properties of a window. Typically     //
//   added to an MDI area as a child window.                       //
/////////////////////////////////////////////////////////////////////

#include "PropertiesWindow.h"
#include "StringRenderer.h"

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

/*-----------------------------------------------------------------+
 | Helper method that writes the string name and value to the      |
 | streamas as a HTML table row.                                   |
 +-----------------------------------------------------------------*/
void writeField(QTextStream& stream, int num, String name, String value) {
    String row = (num % 2 == 0) ? "even" : "odd";
    stream << "<tr class=\"" << row << "\"><td class=\"name\"><p>"
           << name << "</p></td><td class=\"data\"><p>"
           << value << "</p></td></tr>";
}

/*-----------------------------------------------------------------+
 | Builds the HTML for the properties view.                        |
 +-----------------------------------------------------------------*/
void PropertiesWindow::setupProperties() {
    String htmlString;
    QTextStream stream(&htmlString);

    stream << "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"
              "<html><head><link type=\"text/css\" rel=\"StyleSheet\" href=\"styles/PropertiesWindow.css\"/>"
              "</head><body><table width=\"100%\" height=\"100%\" border=\"1\">"
              "<tr><th>Item</th><th>Value</th></tr>";
    int i = 1;
    writeField(stream, i++, "Window Class", htmlLabel(client->getWindowClass()));
    writeField(stream, i++, "Text/Title", htmlLabel(client->getText()));
    writeField(stream, i++, "Handle", htmlLabel(client->getHandle()));
    if (client->getParent())
        writeField(stream, i++, "Parent Handle", htmlLabel(client->getParent()->getHandle()));
    writeField(stream, i++, "Dimensions", htmlLabel(client->getDimensions()));
    writeField(stream, i++, "Position", htmlLabel(client->getPosition()));
    writeField(stream, i++, "Size", htmlLabel(client->getSize()));
    if (client->isChild())
        writeField(stream, i++, "Relative Dimensions", htmlLabel(client->getRelativeDimensions()));
    writeField(stream, i++, "Client Dimensions", htmlLabel(client->getClientDimensions()));
    writeField(stream, i++, "Style bits", hexString(client->getStyleBits()));
    writeField(stream, i++, "Styles", htmlLabel(client->getStandardStyles()));
    writeField(stream, i++, "Extended Style bits", hexString(client->getExStyleBits()));
    writeField(stream, i++, "Extended Styles", htmlLabel(client->getExtendedStyles()));
    writeField(stream, i++, "Owner Process", htmlLabel(client->getProcess()->getFilePath()));
    writeField(stream, i++, "Owner Process ID", htmlLabel(client->getProcessId()));
    writeField(stream, i++, "Owner Thread ID", htmlLabel(client->getThreadId()));
    stream << "</table></body></html>";

    mainTextEdit->setText(htmlString);
    setWindowTitle(tr("Window Properties - ")+stringLabel(client->getHandle()));
}

/*-----------------------------------------------------------------+
 | This just forwards the signal on with this client window.       |
 +-----------------------------------------------------------------*/
void PropertiesWindow::locateActionTriggered() {
    emit locateWindow(client);
}

/*-----------------------------------------------------------------+
 | The window has changed, set the properties again.               |
 +-----------------------------------------------------------------*/
void PropertiesWindow::update() {
    setupProperties();
}
