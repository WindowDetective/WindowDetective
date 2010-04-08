/////////////////////////////////////////////////////////////////////
// File: PreferencesWindow.cpp                                     //
// Date: 6/3/10                                                    //
// Desc: The UI window which displays the app's preferences.       //
/////////////////////////////////////////////////////////////////////

#include "PreferencesWindow.h"
#include "window_detective/main.h"
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
using namespace inspector;

PreferencesWindow::PreferencesWindow(QWidget *parent) :
    QDialog(parent),
    hasHighlightWindowChanged(false) {
    setupUi(this);
    setWindowFlags(Qt::WindowContextHelpButtonHint);
    QPushButton* okButton = dialogButtons->addButton(QDialogButtonBox::Ok);
    QPushButton* applyButton = dialogButtons->addButton(QDialogButtonBox::Apply);

    connect(rbBorder, SIGNAL(clicked()), this, SLOT(borderRadioButtonClicked()));
    connect(rbFilled, SIGNAL(clicked()), this, SLOT(filledRadioButtonClicked()));
    connect(btnChooseFolder, SIGNAL(clicked()), this, SLOT(chooseFolderButtonClicked()));
    connect(slHighlighterTransparency, SIGNAL(valueChanged(int)), this, SLOT(highlightWindowValueChanged()));
    connect(stylesList, SIGNAL(currentRowChanged(int)), this, SLOT(styleListChanged(int)));
    connect(okButton, SIGNAL(clicked()), this, SLOT(applyPreferences()));
    connect(applyButton, SIGNAL(clicked()), this, SLOT(applyPreferences()));
}

PreferencesWindow::~PreferencesWindow() {
}

/*------------------------------------------------------------------+
 | Copies the model data (Settings in this case) to the UI widgets  |
 +------------------------------------------------------------------*/
void PreferencesWindow::copyModelToWindow() {
    // General
    if (Settings::use32bitCursor)
        rb32bitCursor->setChecked(true);
    else
        rb16bitCursor->setChecked(true);
    if (Settings::getOSVersion() < 501)
        rb32bitCursor->setEnabled(false);

    // Window Tree
    chGreyHiddenWindows->setChecked(Settings::greyHiddenWindows);
    spnChangeDuration->setValue(Settings::treeChangeDuration);
    btnCreatedColour->setColour(Settings::itemCreatedColour);
    btnDestroyedColour->setColour(Settings::itemDestroyedColour);

    // Picker
    chPickTransparent->setChecked(Settings::canPickTransparentWindows);

    switch (Settings::highlighterStyle) {
        case Border: rbBorder->click(); break;
        case Filled: rbFilled->click(); break;
        default:     break;  // None selected
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
    txtLogFolder->setText(Settings::logOutputFolder);

    // Styles
    for (int i = 0; i < stylesList->count(); i++) {
        if (stylesList->item(i)->text().toLower() == Settings::appStyle)
            stylesList->setCurrentRow(i, QItemSelectionModel::Select);
    }
}

/*------------------------------------------------------------------+
 | Copies the widget's values to their respective model data.       |
 +------------------------------------------------------------------*/
void PreferencesWindow::copyWindowToModel() {
    // General
    Settings::use32bitCursor = rb32bitCursor->isChecked();

    // Window Tree
    Settings::greyHiddenWindows = chGreyHiddenWindows->isChecked();
    Settings::treeChangeDuration = spnChangeDuration->value();
    Settings::itemCreatedColour = btnCreatedColour->getColour();
    Settings::itemDestroyedColour = btnDestroyedColour->getColour();

    // Picker
    Settings::canPickTransparentWindows = chPickTransparent->isChecked();

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
    if (Settings::enableLogging)
        Logger::getCurrent()->startLoggingToFile();
    else
        Logger::getCurrent()->stopLoggingToFile();

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
 | We need to know when any of the highlight window's properties    |
 | has been changed because it will need to be rebuilt.             |
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
 | Applies the values to the model and saves them. If any           |
 | highlight window values have changed, a signal is emitted to     |
 | tell any HighlightWindows to rebuild themselves.                 |
 +------------------------------------------------------------------*/
void PreferencesWindow::applyPreferences() {
    copyWindowToModel();
    Settings::write();

    if (hasHighlightWindowChanged) {
        emit highlightWindowChanged();
        hasHighlightWindowChanged = false;
    }
}
