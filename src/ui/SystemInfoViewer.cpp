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


/********************************/
/*** SystemColoursModel class ***/
/********************************/

SystemColoursModel::SystemColoursModel(QObject* parent) :
    QAbstractTableModel(parent),
    constants(), defaultColours() {

    QMap<uint,String> colourMap = Resources::getConstants("SystemColours");
    QMap<uint,String>::const_iterator i;

    // Remember initial system colours, so the user can revert back to them
    for (i = colourMap.constBegin(); i != colourMap.constEnd(); i++) {
        COLORREF rgbValue = GetSysColor(i.key());
        defaultColours.insert(i.key(), rgbValue);
    }

    // Build the list of constants. The map isn't used directly, since
    // we want a specific ordering (by id).
    for (i = colourMap.constBegin(); i != colourMap.constEnd(); i++) {
        constants.append(SystemConstant(i.key(), i.value()));
    }
    qSort(constants.begin(), constants.end());
}

/*------------------------------------------------------------------+
| Restores the system colours to their initial values, which were   |
| obtained when this object is first created. As far as i know,     |
| there is no Win API to do this.                                   |
+------------------------------------------------------------------*/
void SystemColoursModel::reset() {
    const uint numColours = defaultColours.size();
    INT* idArray = new INT[numColours];
    COLORREF* valueArray = new COLORREF[numColours];

    uint index = 0;
    QMap<uint,COLORREF>::const_iterator i;
    for (i = defaultColours.constBegin(); i != defaultColours.constEnd(); i++) {
        idArray[index] = i.key();
        valueArray[index] = i.value();
        index++;
    }
    SetSysColors(numColours, idArray, valueArray);
    delete[] idArray;
    delete[] valueArray;

    // Update all data in second and third columns
    emit dataChanged(createIndex(0, 1), createIndex(constants.size(), 2));
}

int SystemColoursModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return constants.size();
}

int SystemColoursModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 3;          // Name, RGB string, actual colour
}

QVariant SystemColoursModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= constants.size() || index.row() < 0) {
        return QVariant();
    }

    SystemConstant constant = constants.at(index.row());
    if (index.column() == 0 && role == Qt::DisplayRole) {
        return constant.name;
    }

    QColor colour = QColorFromCOLORREF(GetSysColor(constant.id));
    if (index.column() == 1 && role == Qt::DisplayRole) {
        return stringLabel(colour);
    }
    else if (index.column() == 2 && role == Qt::BackgroundRole) {
        return colour;
    }

    if (index.column() == 1 && role == Qt::TextAlignmentRole) {
        return Qt::AlignHCenter;
    }

    return QVariant();
}

QVariant SystemColoursModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:  return tr("Name");
            case 1:  return tr("RGB");
            case 2:  return tr("Colour");
        }
    }
    return QVariant();
}

Qt::ItemFlags SystemColoursModel::flags(const QModelIndex& index) const {
    Q_UNUSED(index);
    return Qt::ItemIsEnabled;
}

bool SystemColoursModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    SystemConstant constant = constants.at(index.row());
    if (index.column() == 2 && role == Qt::BackgroundRole) { // Colour value was set
        const INT colourId = constant.id;
        COLORREF rgbValue = COLORREFFromQColor(value.value<QColor>());
        if (!SetSysColors(1, &colourId, &rgbValue)) {
            Logger::osError("Could not set system colour "+constant.name);
        }
        emit dataChanged(index, index);
        return true;
    }
    else {
        return false;
    }
}


/********************************/
/*** SystemMetricsModel class ***/
/********************************/

SystemMetricsModel::SystemMetricsModel(QObject* parent) :
    QAbstractTableModel(parent) {

    // Build the list of constants. The map isn't used directly, since
    // we want a specific ordering (by id).
    QMap<uint,String> metricMap = Resources::getConstants("SystemMetrics");
    QMap<uint,String>::const_iterator i;
    for (i = metricMap.constBegin(); i != metricMap.constEnd(); i++) {
        constants.append(SystemConstant(i.key(), i.value()));
    }
    qSort(constants.begin(), constants.end());
}

int SystemMetricsModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return constants.size();
}

int SystemMetricsModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return 2;          // Name, value
}

QVariant SystemMetricsModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid() || index.row() >= constants.size() || index.row() < 0) {
        return QVariant();
    }

    SystemConstant constant = constants.at(index.row());
    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case 0:  return constant.name;
            case 1:  return stringLabel(GetSystemMetrics(constant.id));
        }
    }

    return QVariant();
}

QVariant SystemMetricsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole)
        return QVariant();
    
    if (orientation == Qt::Horizontal) {
        switch (section) {
            case 0:  return tr("Name");
            case 1:  return tr("Value");
        }
    }
    return QVariant();
}

Qt::ItemFlags SystemMetricsModel::flags(const QModelIndex& index) const {
    Q_UNUSED(index);
    return Qt::ItemIsEnabled;
}


/******************************/
/*** SystemInfoViewer class ***/
/******************************/

SystemInfoViewer::SystemInfoViewer(QWidget* parent):
    QDialog(parent) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if (Settings::stayOnTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    }
    setupUi(this);
    connect(systemColoursTable, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(colourTableDoubleClicked(const QModelIndex&)));
    tabWidget->setCurrentIndex(0);

    SystemMetricsModel* systemMetricsModel = new SystemMetricsModel();
    systemMetricsTable->setModel(systemMetricsModel);

    SystemColoursModel* systemColoursModel = new SystemColoursModel();
    connect(resetColoursButton, SIGNAL(clicked()), systemColoursModel, SLOT(reset()));
    systemColoursTable->setModel(systemColoursModel);

    // Workaround to get columns to resize correctly - http://bugreports.qt.nokia.com/browse/QTBUG-9352
    systemMetricsTable->setVisible(false);  
    systemMetricsTable->resizeColumnsToContents();
    systemMetricsTable->resizeRowsToContents();
    systemMetricsTable->setVisible(true);

    systemColoursTable->setVisible(false);
    systemColoursTable->resizeColumnsToContents();
    systemColoursTable->resizeRowsToContents();
    systemColoursTable->setVisible(true);
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
| Gets various general system information (by calling the           |
| SystemParametersInfo function) and populates the labels.          |
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

void SystemInfoViewer::showEvent(QShowEvent*) {
    readSmartSettings();
    populateGeneralInfo();    // Do this every time the dialog is opened, incase data has changed
}

void SystemInfoViewer::hideEvent(QHideEvent*) {
    writeSmartSettings();
}

/*------------------------------------------------------------------+
| A cell in the colours table was double-clicked.                   |
| If it is the 3rd column, show the colour picker dialog to let the |
| user choose and set a new colour.                                 |
+------------------------------------------------------------------*/
void SystemInfoViewer::colourTableDoubleClicked(const QModelIndex& index) {
    if (index.column() != 2) return;     // Colour cells are in the 3nd column

    QAbstractItemModel* model = systemColoursTable->model();
    QColor currentColour = model->data(index, Qt::BackgroundRole).value<QColor>();
    QColor chosen = QColorDialog::getColor(currentColour, this, tr("Select colour"));
    if (chosen.isValid()) {
        model->setData(index, chosen, Qt::BackgroundRole);
    }
}