/////////////////////////////////////////////////////////////////////
// File: HighlightWindow.h                                         //
// Date: 17/2/10                                                   //
// Desc: This window is created as a "layered" window which is     //
//   transparent to mouse input and is always shown on top of      //
//   every other windows while still allowing those windows to     //
//   show through it. It is meant to be shown over the window      //
//   which needs to be highlighted. Various styles can be used to  //
//   show the highlight. The most basic is a red border of a       //
//   specified thickness, displayed at 100% alpha transparency,    //
//   with the rest of the window invisible (clipped). Another      //
//   style is a fully coloured window with an alpha transparency   //
//   of around 10-20%.                                             //
//   Note: Throughout this class, the term 'client' is used to     //
//   describe the window that this will display info for.          //
/////////////////////////////////////////////////////////////////////

#ifndef HIGHLIGHT_WINDOW_H
#define HIGHLIGHT_WINDOW_H

#include "window_detective/include.h"
#include "ui/InfoWindow.h"
#include "inspector/inspector.h"


// Unique name for highlighter window. "Window Detective - HighlightWindow"
#define HIGHLIGHT_WINDOW_CLASS_NAME  L"WD_HighlightWindow"

enum HighlightStyle {
    Border,
    Filled
};

class HighlightWindow : public QObject {
    Q_OBJECT()
private:
    HWND handle;                   // Handle of this window
    inspector::Window* prevWindow; // Cache last client window highlighted
    InfoWindow* infoWindow;        // Displays info about the highlighted window
    QTimer* flashTimer;            // Timer for flashing this window
    int flashTimes;                // Number of times to flash (show then hide)
public:
    static bool isWindowClassCreated;
    static void createWindowClass();
    static LRESULT CALLBACK wndProc(HWND, UINT, WPARAM, LPARAM);

    HighlightWindow(bool showInfoWindow = false);
    ~HighlightWindow();

    void create();
    void highlight(inspector::Window* window);
    void flash(inspector::Window* window, int interval = 250, int times = 4);
    void moveTo(inspector::Window* window);
    void show();
    void hide();
public slots:
    void update();
private slots:
    void doSingleFlash();
private:
    void createBorderRegion(const QRect& windowRect);
};

#endif   // HIGHLIGHT_WINDOW_H