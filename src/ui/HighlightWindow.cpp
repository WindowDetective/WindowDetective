/////////////////////////////////////////////////////////////////////
// File: HighlightWindow.cpp                                       //
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

// TODO: This will be transparent to mouse, but we also want to detect other
//  transparent windows. So the function to do that will first have to check
//  if the window under cursor is *this* one and if so, either hide it or
//  perform a check which does *not* include transparent windows
//  (ChildWindowFromPointEx with CWP_SKIPTRANSPARENT should be able to do this)
//
//  Also, Vista does some fancy animations when showing/hiding windows. We will
//  need to either disable them for this window or set it's alpha to 0 before
//  showing. Although initial testing on Vista does not seem to show this.

#include "HighlightWindow.h"
#include "window_detective/Settings.h"
#include "window_detective/Logger.h"
using namespace inspector;

bool HighlightWindow::isWindowClassCreated = false;


/**********************/
/*** Static methods ***/
/**********************/

/*------------------------------------------------------------------+
| Creates the window class that any instance of HighlightWindow     |
| will use. It is called the first time a highlight window is       |
| created and can only be called once                               |
+------------------------------------------------------------------*/
void HighlightWindow::createWindowClass() {
    if (HighlightWindow::isWindowClassCreated)
        return;   // Can only be called once

    WNDCLASS wndclass;
    wndclass.style = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc = (WNDPROC)HighlightWindow::wndProc;
    wndclass.cbClsExtra = 0;
    wndclass.cbWndExtra = 0;
    wndclass.hIcon = NULL;
    wndclass.hInstance = GetModuleHandle(NULL);
    wndclass.hCursor = NULL;
    wndclass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wndclass.lpszMenuName = NULL;
    wndclass.lpszClassName = HIGHLIGHT_WINDOW_CLASS_NAME;

    if (RegisterClass(&wndclass)) {
        HighlightWindow::isWindowClassCreated = true;
    }
    else {
        Logger::osError(TR("Could not register highlighter window"));
    }
}

/*------------------------------------------------------------------+
| Window callback procedure for all highlight windows.              |
+------------------------------------------------------------------*/
// TODO: May need to store an list of HighlightWindow which keeps track of
//  any created so that this can get the actual object from the hwnd
LRESULT CALLBACK HighlightWindow::wndProc(HWND hwnd, UINT umsg,
                    WPARAM wParam, LPARAM lParam) {
    /* TODO: Should i do this here or when the object is deleted?
    case WM_CLOSE:
      DestroyWindow(hwnd);
      return 0;*/

    return DefWindowProc(hwnd, umsg, wParam, lParam);
}


/************************/
/*** Instance methods ***/
/************************/

/*------------------------------------------------------------------+
| Constructor                                                       |
+------------------------------------------------------------------*/
HighlightWindow::HighlightWindow(bool showInfoWindow) :
    handle(NULL),
    infoWindow(NULL),
    flashTimer(NULL),
    flashTimes(0),
    prevWindow(NULL) {
    if (showInfoWindow) {
        infoWindow = new InfoWindow();
    }
}

/*------------------------------------------------------------------+
| Destructor                                                        |
+------------------------------------------------------------------*/
HighlightWindow::~HighlightWindow() {
    if (infoWindow) {
        delete infoWindow;
        infoWindow = NULL;
    }
    if (flashTimer) {
        delete flashTimer;
    }
    if (handle && !DestroyWindow(handle)) {
        Logger::osWarning(TR("Could not destroy highlighter window"));
    }
    handle = NULL;
}

/*------------------------------------------------------------------+
| Create the actual window. It will be hidden to start with.        |
+------------------------------------------------------------------*/
void HighlightWindow::create() {
    if (!HighlightWindow::isWindowClassCreated)
        createWindowClass();

    DWORD style = WS_POPUP;
    DWORD exStyle = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW;
    handle = CreateWindowEx(
                exStyle,                // extended style
                HIGHLIGHT_WINDOW_CLASS_NAME,
                L"",                    // window name
                style,                  // style
                0, 0, 0, 0,             // dimensions
                NULL,                   // parent window
                NULL,                   // menu
                GetModuleHandle(NULL),  // instance handle
                NULL);                  // creation data

    if (!handle) {
        Logger::osError(TR("Could not create highlighter window"));
        return;
    }
    update();
}

/*------------------------------------------------------------------+
| Update any window properties from the settings. This is needed    |
| if a the highlight colour or transparency is changed.             |
+------------------------------------------------------------------*/
void HighlightWindow::update() {
    // Set window's background colour.
    // TODO: Is the brush destroyed by the class? If i change it, do i need
    //   to destroy the old one?
    HBRUSH brush = CreateSolidBrush(COLORREFFromQColor(Settings::highlighterColour));
    SetClassLongPtr(handle, GCL_HBRBACKGROUND, (LONG_PTR)brush);

    // Set alpha transparency
    SetLayeredWindowAttributes(handle, 0,
                Settings::highlighterColour.alpha(), LWA_ALPHA);
}

/*------------------------------------------------------------------+
| Position this window over the client window to highlight it.      |
+------------------------------------------------------------------*/
void HighlightWindow::highlight(Window* window) {
    // No need to do anything if it's the same window (as long as that
    // window has not changed it's position)
    if (window == prevWindow)
        return;

    // Hide this window while moving to prevent flicker
    ShowWindow(handle, SW_HIDE);
    moveTo(window);
    show();
    prevWindow = window;
}

/*------------------------------------------------------------------+
| Flashes this window by showing and hiding it over the client.     |
| window.                                                           |
|  interval - the duration of each flash in milliseconds            |
|  times    - number of times to flash on then off                  |
+------------------------------------------------------------------*/
void HighlightWindow::flash(Window* window, int interval, int times) {
    if (!flashTimer) {
        flashTimer = new QTimer();
        flashTimer->setSingleShot(false);
        connect(flashTimer, SIGNAL(timeout()), this, SLOT(doSingleFlash()));
    }
    flashTimes = times;
    flashTimer->start(interval/2);   // 1/2 time for show, 1/2 for hide
    moveTo(window);
}

/*------------------------------------------------------------------+
| Do a single flash by showing or hiding this window. This method   |
| is called by the flash timer each time it fires.                  |
+------------------------------------------------------------------*/
void HighlightWindow::doSingleFlash() {
    static bool isShow = true;
    static int numTimes = 0;   // Times this has been called

    isShow ? show() : hide();
    numTimes++;
    isShow = !isShow;          // Toggle show/hide to "flash" the window
    if (numTimes >= flashTimes * 2) {
        flashTimer->stop();    // Stop timer and reset counter
        numTimes = 0;
    }
}

void HighlightWindow::show() {
    SetWindowPos(handle, HWND_TOPMOST, 0, 0, 0, 0,
            SWP_SHOWWINDOW | SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    if (infoWindow)
        infoWindow->show();
}

void HighlightWindow::hide() {
    ShowWindow(handle, SW_HIDE);
    if (infoWindow)
        infoWindow->hide();
    prevWindow = NULL;
}

/*------------------------------------------------------------------+
| Sets the position of this window to be that of the client window. |
| Note that this function does not show or hide this window.        |
+------------------------------------------------------------------*/
void HighlightWindow::moveTo(Window* window) {
    QRect rect = window->getDimensions();

    SetWindowRgn(handle, NULL, FALSE); // Reset hole region
    SetWindowPos(handle, 0,
        rect.x(), rect.y(), rect.width(), rect.height(),
        SWP_NOACTIVATE | SWP_NOZORDER);

    // Update hole region is using border style
    if (Settings::highlighterStyle == Border) {
        createBorderRegion(rect);
    }
    if (infoWindow)
        infoWindow->moveTo(window);
}

/*------------------------------------------------------------------+
| Creates a hole in the window such that only a border of the       |
| required width is showing. This must be called every time the     |
| window is moved.                                                  |
+------------------------------------------------------------------*/
void HighlightWindow::createBorderRegion(const QRect& windowRect) {
    HRGN windowRegion, holeRegion, borderRegion;
    int thickness = Settings::highlighterBorderThickness;

    if (windowRect.width() <= thickness*2 ||
        windowRect.height() <= thickness*2)
        return;   // No need to do anything if rect is smaller than border

    windowRegion = CreateRectRgn(0, 0, windowRect.width(), windowRect.height());
    holeRegion = CreateRectRgn(thickness, thickness,
                    windowRect.width() - thickness,
                    windowRect.height() - thickness);
    borderRegion = CreateRectRgn(0, 0, 0, 0);

    CombineRgn(borderRegion, holeRegion, windowRegion, RGN_XOR);
    SetWindowRgn(handle, borderRegion, TRUE);
}