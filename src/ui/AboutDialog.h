/////////////////////////////////////////////////////////////////////
// File: AboutDialog.h                                             //
// Date: 15/6/10                                                   //
// Desc: The dialog shown in Help > About Window Detective.        //
//    Includes version number, description and Qt info.            //
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

#ifndef ABOUT_DIALOG_H
#define ABOUT_DIALOG_H

#include <QtGui\QtGui>

class AboutDialog : public QDialog {
private:
    QVBoxLayout* dialogLayout;
    QFrame* mainFrame;
    QHBoxLayout* mainFrameLayout;
    QFrame* leftFrame;
    QVBoxLayout* leftFrameLayout;
    QLabel* iconLabel;
    QFrame* centerFrame;
    QVBoxLayout* centerFrameLayout;
    QLabel* titleLabel;
    QLabel* versionLabel;
    QLabel* descriptionLabel;
    QPlainTextEdit* licenseText;
    QLabel* websiteLabel;
    QFrame* bottomFrame;
    QHBoxLayout* bottomFrameLayout;
    QSpacerItem* horizontalSpacer;
    QLabel* aboutQtLabel;
    QPushButton* aboutQtButton;
    QPushButton* closeButton;

public:
    AboutDialog(QWidget* parent = 0);

    void setupUi();
    void retranslateUi();
};

QT_END_NAMESPACE

#endif // ABOUT_DIALOG_H
