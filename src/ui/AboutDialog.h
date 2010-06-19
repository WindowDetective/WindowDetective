/////////////////////////////////////////////////////////////////////
// File: AboutDialog.h                                             //
// Date: 15/6/10                                                   //
// Desc: The dialog shown in Help > About Window Detective.        //
//    Includes version number, description and Qt info.            //
/////////////////////////////////////////////////////////////////////

#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>


class AboutDialog : public QDialog {
private:
    QLabel* iconLabel;
    QLabel* titleLabel;
    QLabel* versionLabel;
    QLabel* descriptionLabel;
    QLabel* websiteLabel;
    QLabel* aboutQtLabel;
    QPushButton *aboutQtButton;
    QPushButton *closeButton;

public:
    AboutDialog(QWidget* parent = 0);

    void setupUi();
    void retranslateUi();
};

QT_END_NAMESPACE

#endif // ABOUT_DIALOG_H
