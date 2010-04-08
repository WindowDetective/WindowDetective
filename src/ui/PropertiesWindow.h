/////////////////////////////////////////////////////////////////////
// File: PropertiesWindow.h                                        //
// Date: 23/3/10                                                   //
// Desc: Used to display the properties of a window. Typically     //
//   added to an MDI area as a child window.                       //
//   Note: Throughout this class, the term 'client' is used to     //
//   describe the window that this will display info for.          //
/////////////////////////////////////////////////////////////////////

#ifndef PROPERTY_WINDOW_H
#define PROPERTY_WINDOW_H

#include "window_detective/include.h"
#include "inspector/inspector.h"
#include "ui_PropertiesWindow.h"
using namespace inspector;

class PropertiesWindow : public QMainWindow, private Ui::PropertiesWindow {
    Q_OBJECT
private:
    Window* client;
public:
    PropertiesWindow(Window* window, QWidget* parent = 0);
    ~PropertiesWindow();

    void setupProperties();
signals:
    void locateWindow(Window*);
private slots:
    void locateActionTriggered();
    void update();
};

#endif   // PROPERTY_WINDOW_H