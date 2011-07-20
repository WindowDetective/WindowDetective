/////////////////////////////////////////////////////////////////////
// File: PreferencesWindow.cpp                                     //
// Date: 6/3/10                                                    //
// Desc: The UI window which displays the app's preferences.       //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010-2011 XTAL256

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

#include "PreferencesWindow.h"
#include "window_detective/main.h"
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
using namespace inspector;

PreferencesWindow::PreferencesWindow(QWidget *parent) :
    QDialog(parent),
    hasHighlightWindowChanged(false),
    hasStayOnTopChanged(false) {
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    if (Settings::stayOnTop) {
        setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    }
    setupUi(this);
    QPushButton* okButton = dialogButtons->addButton(QDialogButtonBox::Ok);
    QPushButton* applyButton = dialogButtons->addButton(QDialogButtonBox::Apply);

    // TODO: This isn't implemented yet, so hide it in the UI
    chPickTransparent->hide();

    connect(rbBorder, SIGNAL(clicked()), this, SLOT(borderRadioButtonClicked()));
    connect(rbFilled, SIGNAL(clicked()), this, SLOT(filledRadioButtonClicked()));
    connect(btnChooseFolder, SIGNAL(clicked()), this, SLOT(chooseFolderButtonClicked()));
    connect(slHighlighterTransparency, SIGNAL(valueChanged(int)), this, SLOT(highlightWindowValueChanged()));
    connect(stylesList, SIGNAL(currentRowChanged(int)), this, SLOT(styleListChanged(int)));
    connect(okButton, SIGNAL(clicked()), this, SLOT(applyPreferences()));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(applyPreferences()));
}

/*------------------------------------------------------------------+
| Copies the model data (Settings in this case) to the UI widgets   |
+------------------------------------------------------------------*/
void PreferencesWindow::copyModelToWindow() {
    // General
    if (Settings::use32bitCursor)
        rb32bitCursor->setChecked(true);
    else
        rb16bitCursor->setChecked(true);

    if (getOSVersion() < 501)
        rb32bitCursor->setEnabled(false);

    switch (Settings::regexType) {
        case QRegExp::RegExp: rbStandardRegex->click(); break;
        case QRegExp::Wildcard: rbWildcardRegex->click(); break;
        case QRegExp::WildcardUnix: rbWildcardUnixRegex->click(); break;
        default:     break;  // None selected - shouldn't happen
    }

    chStayOnTop->setChecked(Settings::stayOnTop);

    // Window Tree
    chGreyHiddenWindows->setChecked(Settings::greyHiddenWindows);
    spnChangeDuration->setValue(Settings::treeChangeDuration);
    btnCreatedColour1->setColour(Settings::itemCreatedColours.first);
    btnCreatedColour2->setColour(Settings::itemCreatedColours.second);
    btnDestroyedColour1->setColour(Settings::itemDestroyedColours.first);
    btnDestroyedColour2->setColour(Settings::itemDestroyedColours.second);
    btnChangedColour1->setColour(Settings::itemChangedColours.first);
    btnChangedColour2->setColour(Settings::itemChangedColours.second);

    // Picker
    chPickTransparent->setChecked(Settings::canPickTransparentWindows);
    chHideWhilePicking->setChecked(Settings::hideWhilePicking);

    switch (Settings::highlighterStyle) {
        case Border: rbBorder->click(); break;
        case Filled: rbFilled->click(); break;
        default:     break;  // None selected - shouldn't happen
    }

    btnHighlighterColour->setColour(Settings::highlighterColour);
    slHighlighterTransparency->setValue(Settings::highlighterColour.alpha());
    spnThickness->setValue(Settings::highlighterBorderThickness);

    QStringList labels = Settings::infoLabels;
    chWindowClass->setChecked(labels.contains("windowClass"));
    chWindowText->setChecked(labels.contains("text"));
    chWindowHandle->setChecked(labels.contains("handle"));
    chParentHandle->setChecked(labels.contains("parentHandle"));
    chWindowDimensions->setChecked(labels.contains("dimensions"));
    chWindowPosition->setChecked(labels.contains("position"));
    chWindowSize->setChecked(labels.contains("size"));

    // Logging
    chLogToFile->setChecked(Settings::enableLogging);
    chEnableBalloon->setChecked(Settings::enableBalloonNotifications);
    txtLogFolder->setText(Settings::logOutputFolder);

    // Styles
    for (int i = 0; i < stylesList->count(); i++) {
        if (stylesList->item(i)->text().toLower() == Settings::appStyle)
            stylesList->setCurrentRow(i, QItemSelectionModel::Select);
    }
}

/*------------------------------------------------------------------+
| Copies the widget's values to their respective model data.        |
+------------------------------------------------------------------*/
void PreferencesWindow::copyWindowToModel() {
    // General
    Settings::use32bitCursor = rb32bitCursor->isChecked();
    
    if (rbStandardRegex->isChecked())
        Settings::regexType = QRegExp::RegExp;
    else if (rbWildcardRegex->isChecked())
        Settings::regexType = QRegExp::Wildcard;
    else if (rbWildcardUnixRegex->isChecked())
        Settings::regexType = QRegExp::WildcardUnix;

    hasStayOnTopChanged = (Settings::stayOnTop != chStayOnTop->isChecked());
    Settings::stayOnTop = chStayOnTop->isChecked();

    // Window Tree
    Settings::greyHiddenWindows = chGreyHiddenWindows->isChecked();
    Settings::treeChangeDuration = spnChangeDuration->value();
    Settings::itemCreatedColours.first = btnCreatedColour1->getColour();
    Settings::itemCreatedColours.second = btnCreatedColour2->getColour();
    Settings::itemDestroyedColours.first = btnDestroyedColour1->getColour();
    Settings::itemDestroyedColours.second = btnDestroyedColour2->getColour();
    Settings::itemChangedColours.first = btnChangedColour1->getColour();
    Settings::itemChangedColours.second = btnChangedColour2->getColour();

    // Picker
    Settings::canPickTransparentWindows = chPickTransparent->isChecked();
    Settings::hideWhilePicking = chHideWhilePicking->isChecked();

    if (rbBorder->isChecked())
        Settings::highlighterStyle = Border;
    else if (rbFilled->isChecked())
        Settings::highlighterStyle = Filled;

    Settings::highlighterColour = btnHighlighterColour->getColour();
    Settings::highlighterColour.setAlpha(slHighlighterTransparency->value());
    Settings::highlighterBorderThickness = spnThickness->value();

    Settings::infoLabels.clear();
    if (chWindowClass->isChecked())
        Settings::infoLabels.append("windowClass");
    if (chWindowText->isChecked())
        Settings::infoLabels.append("text");
    if (chWindowHandle->isChecked())
        Settings::infoLabels.append("handle");
    if (chParentHandle->isChecked())
        Settings::infoLabels.append("parentHandle");
    if (chWindowDimensions->isChecked())
        Settings::infoLabels.append("dimensions");
    if (chWindowPosition->isChecked())
        Settings::infoLabels.append("position");
    if (chWindowSize->isChecked())
        Settings::infoLabels.append("size");

    // Logging
    Settings::enableLogging = chLogToFile->isChecked();
    Settings::logOutputFolder = txtLogFolder->text();
    Settings::enableBalloonNotifications = chEnableBalloon->isChecked();
    if (Settings::enableLogging)
        Logger::current()->startLoggingToFile();
    else
        Logger::current()->stopLoggingToFile();

    // Styles
    Settings::appStyle = stylesList->currentItem()->text().toLower();
    setAppStyle(Settings::appStyle);
}


/**********************/
/*** Event handlers ***/
/**********************/

void PreferencesWindow::showEvent(QShowEvent*) {
    copyModelToWindow();
}

void PreferencesWindow::borderRadioButtonClicked() {
    // Set a sensible transparency to use for border style
    if (slHighlighterTransparency->value() < 128)
        slHighlighterTransparency->setValue(255);
}

void PreferencesWindow::filledRadioButtonClicked() {
    // Set a sensible transparency to use for filled style
    if (slHighlighterTransparency->value() > 200)
        slHighlighterTransparency->setValue(64);
}

void PreferencesWindow::chooseFolderButtonClicked() {
    String folder = QFileDialog::getExistingDirectory(this,
                        tr("Select a folder to write the log to"),
                        QDir::homePath());
    if (!folder.isEmpty())
        txtLogFolder->setText(folder);
}

/*------------------------------------------------------------------+
| We need to know when any of the highlight window's properties     |
| has been changed because it will need to be rebuilt.              |
+------------------------------------------------------------------*/
void PreferencesWindow::highlightWindowValueChanged() {
    hasHighlightWindowChanged = true;
}

void PreferencesWindow::styleListChanged(int index) {
    String styleName = stylesList->item(index)->text().toLower();
    if (styleName != "native" && styleName != "custom") {
        styleSampleLabel->setPixmap(QPixmap(":/img/sample_" + styleName + ".png"));
    }
    else {
        styleSampleLabel->setPixmap(QPixmap());
    }
}

/*------------------------------------------------------------------+
| Applies the values to the model and saves them. If any            |
| highlight window values have changed, a signal is emitted to      |
| tell any HighlightWindows to rebuild themselves.                  |
+------------------------------------------------------------------*/
void PreferencesWindow::applyPreferences() {
    copyWindowToModel();
    Settings::write();

    if (hasStayOnTopChanged) {
        emit stayOnTopChanged(Settings::stayOnTop);
        hasStayOnTopChanged = false;
    }

    if (hasHighlightWindowChanged) {
        emit highlightWindowChanged();
        hasHighlightWindowChanged = false;
    }
}
