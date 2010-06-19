/////////////////////////////////////////////////////////////////////
// File: AboutDialog.cpp                                           //
// Date: 15/6/10                                                   //
// Desc: The dialog shown in Help > About Window Detective.        //
//    Includes version number, description and Qt info.            //
/////////////////////////////////////////////////////////////////////

#include "AboutDialog.h"
#include "window_detective/resource.h"

AboutDialog::AboutDialog(QWidget* parent):
    QDialog(parent) {
    setupUi();
    connect(aboutQtButton, SIGNAL(clicked()), qApp, SLOT(aboutQt()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(reject()));
}

void AboutDialog::setupUi() {
    resize(440, 260);
    setMinimumSize(440, 260);
    setMaximumSize(440, 260);
    iconLabel = new QLabel(this);
    iconLabel->setGeometry(QRect(9, 9, 128, 128));
    iconLabel->setMinimumSize(QSize(128, 128));
    iconLabel->setMaximumSize(QSize(128, 128));
    iconLabel->setPixmap(QPixmap(QString::fromUtf8(":/img/window_detective_large.png")));
    titleLabel = new QLabel(this);
    titleLabel->setGeometry(QRect(160, 20, 241, 16));
    QFont font;
    font.setPointSize(10);
    font.setBold(true);
    font.setWeight(75);
    titleLabel->setFont(font);
    versionLabel = new QLabel(this);
    versionLabel->setGeometry(QRect(160, 60, 241, 16));
    descriptionLabel = new QLabel(this);
    descriptionLabel->setGeometry(QRect(160, 80, 241, 81));
    descriptionLabel->setAlignment(Qt::AlignLeading|Qt::AlignLeft|Qt::AlignTop);
    descriptionLabel->setWordWrap(true);
    websiteLabel = new QLabel(this);
    websiteLabel->setGeometry(QRect(160, 170, 241, 16));
    websiteLabel->setTextInteractionFlags(Qt::LinksAccessibleByMouse|Qt::LinksAccessibleByKeyboard);
    websiteLabel->setOpenExternalLinks(true);
    websiteLabel->setTextFormat(Qt::RichText);
    aboutQtLabel = new QLabel(this);
    aboutQtLabel->setGeometry(QRect(20, 230, 215, 16));
    aboutQtButton = new QPushButton(this);
    aboutQtButton->setGeometry(QRect(260, 227, 75, 23));
    closeButton = new QPushButton(this);
    closeButton->setGeometry(QRect(345, 227, 75, 23));

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
}
