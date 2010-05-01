/////////////////////////////////////////////////////////////////////
// File: InfoWindow.cpp                                            //
// Date: 2/3/10                                                    //
// Desc: This window is shown in the top left corner of a window   //
//   which is being highlighted. It's purpose is to display quick  //
//   info on the window such as it's title, window class and       //
//   position.                                                     //
/////////////////////////////////////////////////////////////////////


#include "InfoWindow.h"
#include "window_detective/Settings.h"
#include "StringRenderer.h"
using namespace inspector;


QMap<String,String> InfoWindow::infoLabelMap;

#define HTML_START  "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">"\
                    "<html><head><link type=\"text/css\" rel=\"StyleSheet\" href=\"styles/InfoWindow.css\"/>"\
                    "</head><body><table width=\"100%\" height=\"100%\">"
#define HTML_END    "</table></body></html>"

/*------------------------------------------------------------------+
 | Creates the map of window variable names to their display labels |
 +------------------------------------------------------------------*/
void InfoWindow::buildInfoLabels() {
    infoLabelMap.insert("windowClass", "Class");
    infoLabelMap.insert("text", "Text");
    infoLabelMap.insert("handle", "HWND");
    infoLabelMap.insert("dimensions", "Rect");
    infoLabelMap.insert("position", "Pos");
    infoLabelMap.insert("size", "Size");
    infoLabelMap.insert("parentHandle", "Parent HWND");
}

/*------------------------------------------------------------------+
 | Constructor                                                      |
 +------------------------------------------------------------------*/
InfoWindow::InfoWindow(QWidget* parent) : QLabel(parent) {
    // Make native window cos' we need it's handle
    setAttribute(Qt::WA_NativeWindow);

    // We want this fully opaque but still transparent to mouse
    DWORD style = WS_POPUP;
    DWORD exStyle = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW;
    SetWindowLongPtr(this->winId(), GWL_STYLE, (LONG_PTR)style);
    SetWindowLongPtr(this->winId(), GWL_EXSTYLE, (LONG_PTR)exStyle);
    SetLayeredWindowAttributes(this->winId(), 0, 255, LWA_ALPHA);
}

/*------------------------------------------------------------------+
 | Moves this window to the top-left corner of the given window,    |
 | ensuring there is enough room for it, and fills the table with   |
 | info for the given window.                                       |
 +------------------------------------------------------------------*/
void InfoWindow::moveTo(Window* window) {
    this->client = window;
    setInfo();

    QRect rect = calcBestDimensions();
    SetWindowPos(this->winId(), 0,
        rect.x(), rect.y(), rect.width(), rect.height(),
        SWP_NOACTIVATE | SWP_NOZORDER);
}

void InfoWindow::show() {
    // FIXME: We want to show this not active, but SetWindowPos does not
    //  seem to work. Strangely enough, calling the super method fixes
    //  the issue, even though QLabel::show() should activate the window.
    SetWindowPos(this->winId(), HWND_TOPMOST, 0, 0, 0, 0,
            SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    QLabel::show();
}

void InfoWindow::hide() {
    ShowWindow(this->winId(), SW_HIDE);
}

/*------------------------------------------------------------------+
 | Sets the info text for this window from the client window.       |
 +------------------------------------------------------------------*/
void InfoWindow::setInfo() {
    String htmlString;
    QTextStream stream(&htmlString);

    stream << HTML_START;
    // Set table data (this bit's kinda messy)
    for (int i = 0; i < Settings::infoLabels.size(); i++) {
        String dataString, label = Settings::infoLabels[i];

        /* TODO: See if i can get this working...
        QVariant value = client->property(label.toUtf8().data());
        dataString = value.toString();*/

        if (label == "windowClass") {
            dataString = stringLabel(client->getWindowClass());
        }
        else if (label == "text") {
            dataString = stringLabel(client->getText());
        }
        else if (label == "handle") {
            dataString = stringLabel(client->getHandle());
        }
        else if (label == "parentHandle") {
            Window* parent = client->getParent();
            dataString = stringLabel(parent ? parent->getHandle() : 0);
        }
        else if (label == "dimensions") {
            dataString = stringLabel(client->getDimensions());
        }
        else if (label == "position") {
            dataString = stringLabel(client->getPosition());
        }
        else if (label == "size") {
            dataString = stringLabel(client->getSize());
        }

        String row = (i % 2 == 0) ? "even" : "odd";
        stream << "<tr class=\"" << row << "\"><td class=\"name\"><p>"
               << infoLabelMap[label] << "</p></td><td class=\"data\"><p>"
               << dataString << "</p></td></tr>";
    }

    stream << HTML_END;
    this->setText(htmlString);
}

/*------------------------------------------------------------------+
 | Calculates the position to place this window such that it fits   |
 | on screen and does not overlap too much of the client window.    |
 +------------------------------------------------------------------*/
QRect InfoWindow::calcBestDimensions() {
    QSize size = this->sizeHint();

    // Up to 10px inset for border style, 2px otherwise
    const int inset = (Settings::highlighterStyle == Border) ?
                    min(Settings::highlighterBorderThickness, 10) : 2;
    QRect windowRect = client->getDimensions();
    QRect desktopRect = QApplication::desktop()->screenGeometry(windowRect.topLeft());
    QPoint bestPos;

    if (windowRect.x() + size.width() > desktopRect.right())
        bestPos.setX(windowRect.x() - size.width());
    else
        if (size.width() > windowRect.width() - 100)
            if (windowRect.x() - size.width() < desktopRect.left())
                bestPos.setX(windowRect.x() + windowRect.width());
            else
                bestPos.setX(windowRect.x() - size.width());
        else
            bestPos.setX(windowRect.x() + inset);

    if (windowRect.y() + size.height() > desktopRect.bottom())
        bestPos.setY(windowRect.y() - size.height());
    else
        if (size.height() > windowRect.height() - 100)
            if (windowRect.y() - size.height() < desktopRect.top())
                bestPos.setY(windowRect.y() + windowRect.height());
            else
                bestPos.setY(windowRect.y() - size.height());
        else
            bestPos.setY(windowRect.y() + inset);

    return QRect(bestPos, size);
}
