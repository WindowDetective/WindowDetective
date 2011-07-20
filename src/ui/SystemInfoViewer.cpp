/////////////////////////////////////////////////////////////////////
// File: SystemInfoViewer.cpp                                      //
// Date: 1/6/11                                                    //
// Desc: Dialog to list all system information obtained by the     //
//   GetSystemMetrics WinAPI function. The name of each metric is  //
//   the SM_* constant defined in the WinAPI.                      //
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

#include "SystemInfoViewer.h"
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
#include "window_detective/StringFormatter.h"
#include "window_detective/QtHelpers.h"

// TODO: Similarly to the window hyperlinks i will do in property windows,
//   system colour names should also have a hyperlink which opens this dialog

SystemInfoViewer::SystemInfoViewer(QWidget* parent):
    QDialog(parent),
    defaultSystemColours() {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if (Settings::stayOnTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    }
    setupUi(this);
    connect(systemColoursTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(colourCellDoubleClicked(int,int)));
    connect(resetColoursButton, SIGNAL(clicked()), this, SLOT(resetSystemColours()));
    tabWidget->setCurrentIndex(0);

    // Remember initial system colours, so the user can revert back to them
    QMap<uint,String> colourConstants = Resources::getConstants("SystemColours");
    QMap<uint,String>::const_iterator i;
    for (i = colourConstants.constBegin(); i != colourConstants.constEnd(); i++) {
        COLORREF rgbValue = GetSysColor(i.key());
        defaultSystemColours.insert(i.key(), rgbValue);
    }
}

void SystemInfoViewer::readSmartSettings() {
    // If the settings don't exist, don't try to read them.
    // It will only mess up the window positions by defaulting to 0
    if (!Settings::isAppInstalled() ||
        !SmartSettings::subKeyExist("systemInfoViewer"))
        return;

    SmartSettings settings;
    settings.setSubKey("systemInfoViewer");

    // Window geometry
    bool shouldMaximize = settings.read<int>("isMaximized");
    int x = settings.read<int>("x");
    int y = settings.read<int>("y");
    int width = settings.read<int>("width");
    int height = settings.read<int>("height");
    move(x, y);
    resize(width, height);
    if (shouldMaximize)
        showMaximized();

    // Tab index
    tabWidget->setCurrentIndex(settings.read<int>("tabIndex"));
}

void SystemInfoViewer::writeSmartSettings() {
    if (!Settings::isAppInstalled()) return;
    SmartSettings settings;

    // Main window geometry
    settings.setSubKey("systemInfoViewer");
    settings.write<bool>("isMaximized", isMaximized());
    if (!isMaximized()) {   // Only remember un-maximised pos
        settings.writeWindowPos("x", x());
        settings.writeWindowPos("y", y());
        settings.writeWindowPos("width", width());
        settings.writeWindowPos("height", height());
    }

    // Tab index
    settings.write<int>("tabIndex", tabWidget->currentIndex());
}

/*------------------------------------------------------------------+
| Populates all data.                                               |
+------------------------------------------------------------------*/
void SystemInfoViewer::populateData() {
    populateGeneralInfo();
    populateSystemMetrics();
    populateSystemColours();
}

/*------------------------------------------------------------------+
| Gets various general system information (by calling the           |
| SystemParametersInfo function) and populates the text widget with |
| this info, formatted nicely.                                      |
+------------------------------------------------------------------*/
void SystemInfoViewer::populateGeneralInfo() {
    // TODO: Show info on multiple monitors.
    //  See http://msdn.microsoft.com/en-us/library/dd145072%28v=VS.85%29.aspx

    WCHAR wallpaperPath[MAX_PATH];
    SystemParametersInfoW(SPI_GETDESKWALLPAPER, MAX_PATH, &wallpaperPath, 0);
    wallpaperPathLabel->setText(String::fromWCharArray(wallpaperPath));

    BOOL isFlatMenu = false;
    SystemParametersInfoW(SPI_GETFLATMENU, 0, &isFlatMenu, 0);
    flatMenusLabel->setText(stringLabel((bool)isFlatMenu));

    BOOL isFontSmoothing = false;
    SystemParametersInfoW(SPI_GETFONTSMOOTHING, 0, &isFontSmoothing, 0);
    fontSmoothingLabel->setText(stringLabel((bool)isFontSmoothing));

    ICONMETRICS iconMetrics;
    ZeroMemory(&iconMetrics, sizeof(ICONMETRICS));
    iconMetrics.cbSize = sizeof(ICONMETRICS);
    SystemParametersInfoW(SPI_GETICONMETRICS, sizeof(ICONMETRICS), &iconMetrics, 0);
    iconHorzSpacingLabel->setText(stringLabel(iconMetrics.iHorzSpacing)+" px");
    iconVertSpacingLabel->setText(stringLabel(iconMetrics.iVertSpacing)+" px");
    iconWrapLabel->setText(stringLabel((bool)iconMetrics.iTitleWrap));

    uint ssTimeout = 0;
    SystemParametersInfoW(SPI_GETSCREENSAVETIMEOUT, 0, &ssTimeout, 0);
    screensaverTimeoutLabel->setText(stringLabel(ssTimeout)+" seconds");

    uint caretWidth = 0;
    SystemParametersInfoW(SPI_GETCARETWIDTH, 0, &caretWidth, 0);
    caretWidthLabel->setText(stringLabel(caretWidth)+" px");

    BOOL isHotTracking = false;
    SystemParametersInfoW(SPI_GETHOTTRACKING, 0, &isHotTracking, 0);
    hotTrackingLabel->setText(stringLabel((bool)isHotTracking));

    BOOL isActiveWindowTracking = false;
    SystemParametersInfoW(SPI_GETACTIVEWINDOWTRACKING, 0, &isActiveWindowTracking, 0);
    windowTrackingLabel->setText(stringLabel((bool)isActiveWindowTracking));
}

/*------------------------------------------------------------------+
| Gets all system metrics (by calling the GetSystemMetrics function)|
| and populates the table widget with this data.                    |
+------------------------------------------------------------------*/
void SystemInfoViewer::populateSystemMetrics() {
    QMap<uint,String> metricConstants = Resources::getConstants("SystemMetrics");
    QMap<uint,String>::const_iterator i;
    uint index = 0;

    systemMetricsTable->setRowCount(metricConstants.size());
    for (i = metricConstants.constBegin(); i != metricConstants.constEnd(); i++) {
        uint metricValue = GetSystemMetrics(i.key());
        systemMetricsTable->setItem(index, 0, new QTableWidgetItem(i.value()));
        systemMetricsTable->setItem(index, 1, new QTableWidgetItem(stringLabel(metricValue)));
        index++;
    }
    systemMetricsTable->setVisible(false);  // Workaround to get columns to resize correctly - http://bugreports.qt.nokia.com/browse/QTBUG-9352
    systemMetricsTable->resizeColumnsToContents();
    systemMetricsTable->resizeRowsToContents();
    systemMetricsTable->setVisible(true);
}

/*------------------------------------------------------------------+
| Gets all system colours (by calling the GetSysColor function)     |
| and populates the table widget with this data.                    |
+------------------------------------------------------------------*/
void SystemInfoViewer::populateSystemColours() {
    QMap<uint,String> colourConstants = Resources::getConstants("SystemColours");
    QMap<uint,String>::const_iterator i;
    uint index = 0;

    systemColoursTable->setRowCount(colourConstants.size());
    for (i = colourConstants.constBegin(); i != colourConstants.constEnd(); i++) {
        COLORREF rgbValue = GetSysColor(i.key());
        QColor colour = QColorFromCOLORREF(rgbValue);

        QTableWidgetItem* colourItem = new QTableWidgetItem("");
        colourItem->setBackgroundColor(colour);

        String rgbString;
        QTextStream stream(&rgbString);
        stream << String::number(colour.red()) << ", "
               << String::number(colour.green()) << ", "
               << String::number(colour.blue());
        QTableWidgetItem* rgbItem = new QTableWidgetItem(rgbString);
        rgbItem->setTextAlignment(Qt::AlignCenter);

        systemColoursTable->setItem(index, 0, new QTableWidgetItem(i.value()));
        systemColoursTable->setItem(index, 1, rgbItem);
        systemColoursTable->setItem(index, 2, colourItem);
        index++;
    }
    systemColoursTable->setVisible(false);  // See populateSystemMetrics
    systemColoursTable->resizeColumnsToContents();
    systemColoursTable->resizeRowsToContents();
    systemColoursTable->setVisible(true);
}

/*------------------------------------------------------------------+
| Choose and set a new colour for the cell at the given row.        |
| A colour picker dialog will be shown to get a new colour.         |
+------------------------------------------------------------------*/
void SystemInfoViewer::chooseColour(int row) {
    QMap<uint,String> colourConstants = Resources::getConstants("SystemColours");
    QTableWidgetItem* nameItem = systemColoursTable->item(row, 0);
    QTableWidgetItem* rgbItem = systemColoursTable->item(row, 1);
    QTableWidgetItem* colourItem = systemColoursTable->item(row, 2);

    QColor chosen = QColorDialog::getColor(colourItem->backgroundColor(), this, tr("Select colour"));
    if (chosen.isValid()) {
        // Do a reverse look-up of the constants table to find the index of the constant who's
        // name is in the cell. Not very nice way of doing it, but it should work.
        const INT index = colourConstants.key(nameItem->text());
        COLORREF rgbValue = COLORREFFromQColor(chosen);
        if (SetSysColors(1, &index, &rgbValue)) {
            String rgbString;
            QTextStream stream(&rgbString);
            stream << String::number(chosen.red()) << ", "
                   << String::number(chosen.green()) << ", "
                   << String::number(chosen.blue());
            rgbItem->setText(rgbString);
            colourItem->setBackgroundColor(chosen);
        }
        else {
            Logger::osError("Could not set system colour "+nameItem->text());
        }
    }
}

/*------------------------------------------------------------------+
| Removes data from the UI. It will then need to be refreshed by    |
| calling populateData().                                           |
+------------------------------------------------------------------*/
void SystemInfoViewer::clearData() {
    systemMetricsTable->clearContents();
    systemColoursTable->clearContents();
}

/*------------------------------------------------------------------+
| Restores the system colours to their initial values, which were   |
| obtained when this object is first created. As far as i know,     |
| there is no Win API to do this.                                   |
+------------------------------------------------------------------*/
void SystemInfoViewer::resetSystemColours() {
    const uint numColours = defaultSystemColours.size();
    INT* idArray = new INT[numColours];
    COLORREF* valueArray = new COLORREF[numColours];

    uint index = 0;
    QMap<uint,COLORREF>::const_iterator i;
    for (i = defaultSystemColours.constBegin(); i != defaultSystemColours.constEnd(); i++) {
        idArray[index] = i.key();
        valueArray[index] = i.value();
        index++;
    }
    SetSysColors(numColours, idArray, valueArray);

    delete[] idArray;
    delete[] valueArray;

    // Update UI
    systemColoursTable->clearContents();
    populateSystemColours();
}


/**********************/
/*** Event handlers ***/
/**********************/

void SystemInfoViewer::showEvent(QShowEvent*) {
    readSmartSettings();
    populateData();    // Do this every time the dialog is opened, incase data has changed
}

void SystemInfoViewer::hideEvent(QHideEvent*) {
    writeSmartSettings();
    clearData();  // Data will be re-loaded next time the window is shown, so no need to keep it around
}

/*------------------------------------------------------------------+
| A cell in the colours table was double-clicked.                   |
+------------------------------------------------------------------*/
void SystemInfoViewer::colourCellDoubleClicked(int row, int column) {
    if (column != 2) return;     // Colour cells are in the 3nd column
    chooseColour(row);
}