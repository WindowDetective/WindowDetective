/////////////////////////////////////////////////////////////////////
// File: SetPropertiesDialog.h                                     //
// Date: 24/3/10                                                   //
// Desc: Used to set the properties of a window. It has a number   //
//   of tabs to group the different properties and a "Set" button  //
//   to apply any changes to the real window.                      //
//   This dialog is modal, since there can only be one per window  //
//   and we also don't want the user changing anything else while  //
//   this is open, otherwise it might disrupt the window's state.  //
/////////////////////////////////////////////////////////////////////

#ifndef SET_PROPERTIES_DIALOG_H
#define SET_PROPERTIES_DIALOG_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "ui_SetPropertiesDialog.h"
using namespace inspector;


#define STANDARD_STYLE_HEADER  " --- Standard Styles --------------------"
#define EXTENDED_STYLE_HEADER  " --- Extended Styles --------------------"
#define HEADER_COLOUR          (QColor(100, 100, 150))

class SetPropertiesDialog : public QDialog, private Ui::SetPropertiesDialog {
    Q_OBJECT
private:
    Window* client;
    QMap<QWidget*, QVariant> lastValues;
    bool isModifyingList;
public:
    SetPropertiesDialog(Window* window, QWidget* parent = 0);
    ~SetPropertiesDialog();

    void copyModelToWindow();
    void copyWindowToModel();
    void showAtTab(int index);
private slots:
    void propertyChanged();
    void dimensionTextChanged();
    void posOrSizeTextChanged();
    void updateStylesList();
    void styleItemChanged(QListWidgetItem*);
    void styleItemSelectionChanged(QListWidgetItem*);
    void styleItemDoubleClicked(QListWidgetItem*);
    void setProperties();
private:
    void buildStylesList();
    QList<int> parseValueString(const String& str);
    void rememberLastValues();
    bool hasChanged(QLineEdit* widget);
    bool hasChanged(QAbstractButton* widget);
    bool hasChanged(QSpinBox* widget);
};

#endif   // SET_PROPERTIES_DIALOG_H