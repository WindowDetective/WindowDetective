/////////////////////////////////////////////////////////////////////
// File: FindDialog.cpp                                            //
// Date: 15/3/10                                                   //
// Desc: The find window. Allows users to search for a window by a //
//   number of different criteria, including name, handle, style.  //
//   If only one window is found, it is selected in the window     //
//   tree. If multiple windows are found, they are displayed in a  //
//   separate list window.                                         //
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

#include "FindDialog.h"
#include "inspector/inspector.h"
#include "inspector/WindowManager.h"
#include "inspector/SearchCriteria.h"
#include "window_detective/Settings.h"
#include "SearchResultsWindow.h"
using namespace inspector;

FindDialog::FindDialog(QWidget* parent) :
    QDialog(parent) {
    setupUi(this);
    QPushButton* findButton = dialogButtons->addButton(tr("&Find"), QDialogButtonBox::AcceptRole);

    // TODO: The advanced tab can be done after the first release.
    //  It will have criteria which can be added and combined using boolean
    //  operators (AND, OR). Each criteria will have three input widgets:
    //  field, operation, value
    //  e.g. <text> <equals> <blah>
    tabWidget->removeTab(1);

    connect(rbWindowText, SIGNAL(clicked()), this, SLOT(windowTextSelected()));
    connect(rbHandle, SIGNAL(clicked()), this, SLOT(handleSelected()));
    connect(rbWindowClass, SIGNAL(clicked()), this, SLOT(windowClassSelected()));
    connect(findButton, SIGNAL(clicked()), this, SLOT(findButtonClicked()));

    rbWindowText->click();
}

void FindDialog::openResultsWindow(WindowList windows,
                                   SearchCriteria searchCriteria) {
    SearchResultsWindow* resultsWindow = new SearchResultsWindow();
    resultsWindow->setAttribute(Qt::WA_DeleteOnClose);
    resultsWindow->openOn(windows, searchCriteria);
}

void FindDialog::readSmartSettings() {
    // If the settings don't exist, don't try to read them.
    // It will only mess up the window positions by defaulting to 0
    if (!Settings::isAppInstalled() ||
        !SmartSettings::subKeyExist("findDialog"))
        return;

    SmartSettings settings;
    settings.setSubKey("findDialog");

    // Window geometry
    int x, y, width, height;
    x = settings.read<int>("x");
    y = settings.read<int>("y");
    width = settings.read<int>("width");
    height = settings.read<int>("height");
    move(x, y);
    resize(width, height);

    // Basic tab
    int index = settings.read<int>("selection");
    switch (index) {
        case 0: rbWindowText->setChecked(true); break;
        case 1: rbHandle->setChecked(true); break;
        case 2: rbWindowClass->setChecked(true); break;
    }
    chCaseSensitive->setChecked(settings.read<bool>("isCaseSensitive"));
    chUseRegex->setChecked(settings.read<bool>("useRegex"));

    // Advanced tab
    // when i get it done
}

void FindDialog::writeSmartSettings() {
    if (!Settings::isAppInstalled()) return;
    SmartSettings settings;
    settings.setSubKey("findDialog");

    // Window geometry
    settings.writeWindowPos("x", x());
    settings.writeWindowPos("y", y());
    settings.writeWindowPos("width", width());
    settings.writeWindowPos("height", height());

    // Basic tab
    int index = 0;
    if(rbWindowText->isChecked())        index = 0;
    else if (rbHandle->isChecked())      index = 1;
    else if (rbWindowClass->isChecked()) index = 2;
    settings.write<int>("selection", index);
    settings.write<bool>("isCaseSensitive", chCaseSensitive->isChecked());
    settings.write<bool>("useRegex", chUseRegex->isChecked());

    // Advanced tab
    // when i get it done
}


/**********************/
/*** Event handlers ***/
/**********************/

void FindDialog::showEvent(QShowEvent*) {
    readSmartSettings();
    cbWindowClass->setList(Resources::windowClasses.values());
    cbWindowClass->clearEditText();
}

void FindDialog::hideEvent(QHideEvent* e) {
    writeSmartSettings();
}

void FindDialog::windowTextSelected() {
    txtWindowText->setFocus(Qt::OtherFocusReason);
    spnHandle->setValue(0);
    cbWindowClass->clearEditText();
}

void FindDialog::handleSelected() {
    spnHandle->setFocus(Qt::OtherFocusReason);
    txtWindowText->setText("");
    cbWindowClass->clearEditText();
}

void FindDialog::windowClassSelected() {
    cbWindowClass->setFocus(Qt::OtherFocusReason);
    txtWindowText->setText("");
    spnHandle->setValue(0);
}

void FindDialog::findButtonClicked() {
    WindowList windows;
    /*** TODO *********************************
     This is just until i get SearchCriteriaItem working
    */
    int type = 0;
    if(rbWindowText->isChecked())        type = 0;
    else if (rbHandle->isChecked())      type = 1;
    else if (rbWindowClass->isChecked()) type = 2;
    SearchCriteria searchCriteria(type, txtWindowText->text(),
                    spnHandle->value(), cbWindowClass->currentText(),
                    chUseRegex->isChecked(), chCaseSensitive->isChecked());
    /******************************************/

    // TODO: Maybe i should make some sort of Search object that takes a
    //   SearchCriteria and returns the results.
    windows = WindowManager::current()->find(searchCriteria);
    openResultsWindow(windows, searchCriteria);
}
