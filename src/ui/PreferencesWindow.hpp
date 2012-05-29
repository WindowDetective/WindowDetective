/////////////////////////////////////////////////////////////////////
// File: PreferencesWindow.hpp                                     //
// Date: 2010-03-06                                                //
// Desc: The UI window which displays the app's preferences.       //
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

#ifndef PREFERENCES_WINDOW_H
#define PREFERENCES_WINDOW_H

#include "window_detective/include.h"
#include "window_detective/Settings.h"
#include "ui/forms/ui_PreferencesWindow.h"

class PreferencesWindow : public QDialog, private Ui::PreferencesWindow {
    Q_OBJECT
private:
    bool hasHighlightWindowChanged;
    bool hasStayOnTopChanged;
    QMenu settingsMenu;

public:
    PreferencesWindow(QWidget *parent = 0);
    ~PreferencesWindow() {}

    void copyModelToWindow();
    void copyWindowToModel();
signals:
    void highlightWindowChanged();
    void stayOnTopChanged(bool shouldStayOnTop);
protected:
    void showEvent(QShowEvent* e);
private slots:
    void borderRadioButtonClicked();
    void filledRadioButtonClicked();
    void highlightWindowValueChanged();
    void chooseFolderButtonClicked();
    void styleListChanged(int index);
    void restoreDefaults();
    void exportSettings();
    void importSettings();
    void applyPreferences();
};

#endif   // PREFERENCES_WINDOW_H