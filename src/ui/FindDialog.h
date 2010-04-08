/////////////////////////////////////////////////////////////////////
// File: FindDialog.h                                              //
// Date: 15/3/10                                                   //
// Desc: The find window. Allows users to search for a window by a //
//   number of different criteria, including name, handle, style.  //
//   If only one window is found, it is selected in the window     //
//   tree. If multiple windows are found, they are displayed in a  //
//   separate list window.                                         //
/////////////////////////////////////////////////////////////////////

#ifndef FIND_DIALOG_H
#define FIND_DIALOG_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "ui_FindDialog.h"
using namespace inspector;

class FindDialog : public QDialog, private Ui::FindDialog {
    Q_OBJECT
public:
    FindDialog(QWidget* parent = 0);
    ~FindDialog();

signals:
    void windowsFound(QList<Window*>);
protected:
    void showEvent(QShowEvent* e);
private slots:
    void windowTextSelected();
    void handleSelected();
    void windowClassSelected();
    void findButtonClicked();
};

#endif   // FIND_DIALOG_H