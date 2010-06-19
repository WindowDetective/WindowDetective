/////////////////////////////////////////////////////////////////////
// File: PreferencesWindow.h                                       //
// Date: 6/3/10                                                    //
// Desc: The UI window which displays the app's preferences.       //
/////////////////////////////////////////////////////////////////////

#ifndef PREFERENCES_WINDOW_H
#define PREFERENCES_WINDOW_H

#include "window_detective/include.h"
#include "forms/ui_PreferencesWindow.h"

class PreferencesWindow : public QDialog, private Ui::PreferencesWindow {
    Q_OBJECT
private:
    bool hasHighlightWindowChanged;

public:
    PreferencesWindow(QWidget *parent = 0);
    ~PreferencesWindow() {}

    void copyModelToWindow();
    void copyWindowToModel();
signals:
    void highlightWindowChanged();
protected:
    void showEvent(QShowEvent* e);
private slots:
    void borderRadioButtonClicked();
    void filledRadioButtonClicked();
    void highlightWindowValueChanged();
    void chooseFolderButtonClicked();
    void styleListChanged(int index);
    void applyPreferences();
};

#endif   // PREFERENCES_WINDOW_H