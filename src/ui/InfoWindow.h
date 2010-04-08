/////////////////////////////////////////////////////////////////////
// File: InfoWindow.h                                              //
// Date: 2/3/10                                                    //
// Desc: This window is shown in the top left corner of a window   //
//   which is being highlighted. It's purpose is to display quick  //
//   info on the window such as it's title, window class and       //
//   position.                                                     //
//   Note: Throughout this class, the term 'client' is used to     //
//   describe the window that this will display info for.          //
/////////////////////////////////////////////////////////////////////

#ifndef INFO_WINDOW_H
#define INFO_WINDOW_H

#include "window_detective/include.h"
#include "inspector/inspector.h"


class InfoWindow : public QLabel {
    Q_OBJECT
private:
    // Maps info labels variable names to display names
    static QMap<String,String> infoLabelMap;

    inspector::Window* client;

public:
    static void buildInfoLabels();

    InfoWindow(QWidget* parent = 0);
    ~InfoWindow() {}

    void moveTo(inspector::Window* window);
    void show();
    void hide();
private:
    void setInfo();
    QRect calcBestDimensions();
};

#endif   // INFO_WINDOW_H