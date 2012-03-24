/////////////////////////////////////////////////////////////////////
// File: SystemInfoViewer.hpp                                      //
// Date: 1/6/11                                                    //
// Desc: Dialog to list all system information obtained by the     //
//   GetSystemMetrics WinAPI function. The name of each metric is  //
//   the SM_* constant defined in the WinAPI.                      //
/////////////////////////////////////////////////////////////////////

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

#ifndef SYSTEM_INFO_VIEWER_H
#define SYSTEM_INFO_VIEWER_H

#include "window_detective/include.h"
#include "ui/forms/ui_SystemInfoViewer.h"


struct SystemConstant {
    uint id;
    String name;

    SystemConstant(uint id, String name) :
        id(id), name(name) {}
    bool operator<(const SystemConstant& other) const { return this->id < other.id; }
};


class SystemColoursModel : public QAbstractTableModel {
    Q_OBJECT
private:
    QList<SystemConstant> constants;
    QHash<uint,COLORREF> defaultColours;

public:
    SystemColoursModel(QObject* parent = 0);
    ~SystemColoursModel() {}

    int rowCount(const QModelIndex&) const { return constants.size(); }
    int columnCount(const QModelIndex&) const { return 3; }   // Name, RGB string, actual colour
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::DisplayRole);
public slots:
    void reset();
};


class SystemMetricsModel : public QAbstractTableModel {
    Q_OBJECT
private:
    QList<SystemConstant> constants;

public:
    SystemMetricsModel(QObject* parent = 0);
    ~SystemMetricsModel() {}

    int rowCount(const QModelIndex&) const { return constants.size(); }
    int columnCount(const QModelIndex&) const { return 2; }   // Name, value
    QVariant data(const QModelIndex& index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
};


class SystemInfoViewer : public QDialog, private Ui::SystemInfoViewer {
    Q_OBJECT
public:
    SystemInfoViewer(QWidget* parent = 0);

protected:
    void readSmartSettings();
    void writeSmartSettings();
    void populateGeneralInfo();
    void showEvent(QShowEvent* e);
    void hideEvent(QHideEvent* e);
public slots:
    void colourTableDoubleClicked(const QModelIndex& index);
};

QT_END_NAMESPACE

#endif // SYSTEM_INFO_VIEWER_H
