/////////////////////////////////////////////////////////////////////
// File: FindDialog.cpp                                            //
// Date: 15/3/10                                                   //
// Desc: The find window. Allows users to search for a window by a //
//   number of different criteria, including name, handle, style.  //
//   If only one window is found, it is selected in the window     //
//   tree. If multiple windows are found, they are displayed in a  //
//   separate list window.                                         //
/////////////////////////////////////////////////////////////////////

#include "FindDialog.h"
#include "inspector/inspector.h"
#include "inspector/WindowManager.h"
#include "inspector/SearchCriteria.h"
#include "window_detective/Settings.h"
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

FindDialog::~FindDialog() {
}


/**********************/
/*** Event handlers ***/
/**********************/

void FindDialog::showEvent(QShowEvent*) {
    cbWindowClass->setList(WindowManager::current()->allWindowClasses.values());
    cbWindowClass->clearEditText();
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
    QList<Window*> windows;
    /*** TODO *********************************
     This is just until i get SearchCriteriaItem working
    */
    int type;
    if(rbWindowText->isChecked()) {
        type = 0;
        if (txtWindowText->text().isEmpty()) return;
    }
    else if (rbHandle->isChecked()) {
        type = 1;
        if (spnHandle->value() == 0) return;
    }
    else if (rbWindowClass->isChecked()) {
        type = 2;
        if (cbWindowClass->currentText().isEmpty()) return;
    }
    SearchCriteria searchCriteria(type, txtWindowText->text(),
                    spnHandle->value(), cbWindowClass->currentText(),
                    chUseRegex->isChecked(), chCaseSensitive->isChecked());
    /******************************************/

    windows = WindowManager::current()->find(searchCriteria);
    if (windows.isEmpty()) {
        QMessageBox::information(this, APP_NAME, "No windows found");
    }
    else {
        emit windowsFound(windows);
    }
}
