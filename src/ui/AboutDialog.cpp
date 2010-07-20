/////////////////////////////////////////////////////////////////////
// File: AboutDialog.cpp                                           //
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

#include "AboutDialog.h"
#include "window_detective/resource.h"

AboutDialog::AboutDialog(QWidget* parent):
    QDialog(parent) {
    setupUi();
    connect(aboutQtButton, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void AboutDialog::setupUi() {
    resize(500, 400);
    setMinimumSize(400, 300);
    setMaximumSize(800, 600);
    dialogLayout = new QVBoxLayout(this);
    dialogLayout->setContentsMargins(3, 3, 3, 3);
    mainFrame = new QFrame(this);
    mainFrameLayout = new QHBoxLayout(mainFrame);
    mainFrameLayout->setContentsMargins(0, 0, 0, 0);
    leftFrame = new QFrame(mainFrame);
    leftFrameLayout = new QVBoxLayout(leftFrame);
    leftFrameLayout->setContentsMargins(-1, 15, -1, -1);
    iconLabel = new QLabel(leftFrame);
    iconLabel->setMinimumSize(QSize(128, 128));
    iconLabel->setPixmap(QPixmap(QString::fromUtf8(":/img/window_detective_large.png")));
    iconLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    leftFrameLayout->addWidget(iconLabel);
    mainFrameLayout->addWidget(leftFrame);
    centerFrame = new QFrame(mainFrame);
    centerFrameLayout = new QVBoxLayout(centerFrame);
    titleLabel = new QLabel(centerFrame);
    QFont font;
    font.setPointSize(10);
    font.setBold(true);
    font.setWeight(75);
    titleLabel->setFont(font);
    titleLabel->setMinimumSize(QSize(0, 30));
    centerFrameLayout->addWidget(titleLabel);
    versionLabel = new QLabel(centerFrame);
    centerFrameLayout->addWidget(versionLabel);
    descriptionLabel = new QLabel(centerFrame);
    descriptionLabel->setWordWrap(true);
    centerFrameLayout->addWidget(descriptionLabel);
    licenseText = new QPlainTextEdit(centerFrame);
    licenseText->setReadOnly(true);
    licenseText->setLineWrapMode(QPlainTextEdit::NoWrap);
    centerFrameLayout->addWidget(licenseText);
    websiteLabel = new QLabel(centerFrame);
    websiteLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::LinksAccessibleByKeyboard);
    websiteLabel->setOpenExternalLinks(true);
    websiteLabel->setTextFormat(Qt::RichText);
    centerFrameLayout->addWidget(websiteLabel);
    mainFrameLayout->addWidget(centerFrame);
    dialogLayout->addWidget(mainFrame);
    bottomFrame = new QFrame(this);
    bottomFrameLayout = new QHBoxLayout(bottomFrame);
    horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    bottomFrameLayout->addItem(horizontalSpacer);
    aboutQtLabel = new QLabel(bottomFrame);
    bottomFrameLayout->addWidget(aboutQtLabel);
    aboutQtButton = new QPushButton(bottomFrame);
    aboutQtButton->setMinimumSize(QSize(75, 0));
    bottomFrameLayout->addWidget(aboutQtButton);
    closeButton = new QPushButton(bottomFrame);
    closeButton->setMinimumSize(QSize(75, 0));
    bottomFrameLayout->addWidget(closeButton);
    dialogLayout->addWidget(bottomFrame);
    dialogLayout->setStretch(0, 10);
    dialogLayout->setStretch(1, 1);

    retranslateUi();
}

void AboutDialog::retranslateUi() {
    setWindowTitle(tr("About") + " " + APP_NAME);
    iconLabel->setText(QString());
    titleLabel->setText(APP_NAME);
    versionLabel->setText(tr("Version") + " " + VERSION_STR);
    descriptionLabel->setText(tr(LONG_DESC));
    QString linkString = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" "\
                    "\"http://www.w3.org/TR/REC-html40/strict.dtd\"><html><body>"\
                    "<a href=\"" WEBSITE "\">" WEBSITE "</a></body></html>";
    websiteLabel->setText(linkString);
    aboutQtLabel->setText(tr("This program is built using the Qt framework"));
    aboutQtButton->setText(tr("About &Qt"));
    closeButton->setText(tr("&Close"));

    QFile licenseFile("copying.txt");
    if (licenseFile.exists()) {
        if (licenseFile.open(QFile::ReadOnly)) {
            QTextStream stream(&licenseFile);
            licenseText->setPlainText(stream.readAll());
        }
        else {
            licenseText->setPlainText(tr("Could not open license file"));
        }
    }
    else {
        licenseText->setPlainText(tr("License file not found"));
    }
}
