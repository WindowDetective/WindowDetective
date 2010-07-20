///////////////////////////////////////////////////////////////////////////////
////////                                                                     //
//     _       _   _             __                                          //
//    | |     / / (_) ____  ____/ /____  _      __                           //
//    | | /| / / / / / __ \/ __  // __ \| | /| / /                           //
//    | |/ |/ / / / / / / / /_/ // /_/ /| |/ |/ /                            //
//    |__/|__/ /_/ /_/ /_/\__,_/ \____/ |__/|__/                             //
//              ____         __              __    _                         //
//             / __ \ ___ __/ /_ ___  ______/ /_  (_)_   __ ___              //
//            / / / // _ \_  __// _ \/ ___/  __/ / /| | / // _ \             //
//           / /_/ //  __// /_ /  __/ /__ / /_  / / | |/ //  __/             //
//          /_____/ \___/ \__/ \___/\___/ \__/ /_/  |___/ \___/              //
//                                                                           //
//    http://windowdetective.sourceforge.net/                                //
//                                                                     ////////
///////////////////////////////////////////////////////////////////////////////

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


#include "main.h"
#include "inspector/WindowManager.h"
#include "inspector/MessageHandler.h"
#include "ui/MainWindow.h"
#include "ui/ActionManager.h"
#include "Settings.h"
#include "Logger.h"
using namespace inspector;

QCursor pickerCursor;
QPalette defaultPalette;   // So we can restore it if need be

HMODULE KernelLibrary = NULL;
HMODULE PsApiLibrary = NULL;

/*------------------------------------------------------------------+
| Main application constructor. Initializes other classes, loads    |
| libraries and sets up various settings.                           |
+------------------------------------------------------------------*/
WindowDetective::WindowDetective(int& argc, char** argv) :
    QApplication(argc, argv) {
    KernelLibrary = LoadLibrary(L"Kernel32.dll");
    PsApiLibrary = LoadLibrary(L"Psapi.dll");

    defaultPalette = QApplication::palette();
    setQuitOnLastWindowClosed(false);
    QDir::setCurrent(appPath());  // TODO: Do i need to set this?
    QApplication::setOrganizationName(APP_NAME);
    QApplication::setApplicationName(APP_NAME);

    Settings::read();
    Logger::initialize();
    giveProcessDebugPrivilege();
    InfoWindow::buildInfoLabels();
    loadPickerCursor();
    ActionManager::initialize();
    WindowManager::initialize();
    MessageHandler::initialize();
    setAppStyle(Settings::appStyle);
}

// Perform any aditional cleanup when the app quits
WindowDetective::~WindowDetective() {
    delete WindowManager::current();
    delete MessageHandler::current();
    delete Logger::current();
    FreeLibrary(KernelLibrary);
    FreeLibrary(PsApiLibrary);
}

/*------------------------------------------------------------------+
| Loads the 'target' cursor from a .cur file. If there is an,       |
| error the system crosshair cursor will be used instead.           |
+------------------------------------------------------------------*/
void loadPickerCursor() {
    HCURSOR hCursor = (HCURSOR)LoadImage(NULL,
            Settings::use32bitCursor ? L"picker_32bit.cur" : L"picker_16bit.cur",
            IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
    if (hCursor) {
        pickerCursor = QCursor(hCursor);
    }
    else {
        // Fall back on crosshair cursor
        pickerCursor = QCursor(Qt::CrossCursor);
        Logger::osError(TR("Could not load cursor. "
                        "Using system crosshair cursor instead."));
    }
}

/*------------------------------------------------------------------+
| Sets the cursor to the 'target' cursor for picking windows        |
+------------------------------------------------------------------*/
void showPickerCursor() {
    QApplication::setOverrideCursor(pickerCursor);
}

void restoreCursor() {
    QApplication::restoreOverrideCursor();
}

/*------------------------------------------------------------------+
| Since there seems to be no way of restoring the default theme,    |
| we have to check the OS version to determine what theme the user  |
| was (hopefully) using.                                            |
+------------------------------------------------------------------*/
void restoreDefaultStyle() {
    int osVersion = getOSVersion();
    QApplication::setPalette(defaultPalette);
    if (osVersion >= 600) {
        QApplication::setStyle("windowsvista");
    }
    else if (osVersion >= 501) {
        QApplication::setStyle("windowsxp");
    }
    else {
        QApplication::setStyle("windows");
    }
}

/*------------------------------------------------------------------+
| Sets the application UI style using either an existing built-in   |
| theme, or the style sheet in the "styles" folder. If the style    |
| is not native, the style's palette will be used instead of the    |
| system's.                                                         |
+------------------------------------------------------------------*/
void setAppStyle(String name) {
    static bool isFirstTime = true;

    if (Settings::appStyle == "native") {
        // No need to reset native style if nothing else has been set yet
        if (!isFirstTime)
            restoreDefaultStyle();
    }
    else if (Settings::appStyle == "custom") {
        if (!isFirstTime) {
            restoreDefaultStyle();
        }
        QFile cssFile("styles/Application.css");
        if (cssFile.exists()) {
            QTextStream stream(&cssFile);
            qApp->setStyleSheet(stream.readAll());
        }
    }
    else {
        QStyle* style = QStyleFactory::create(Settings::appStyle);
        QApplication::setPalette(style->standardPalette());
        QApplication::setStyle(style);
    }
    isFirstTime = false;
}

/*------------------------------------------------------------------+
| Attempts to give the current process debug privilege. With debug  |
| privilege we can do more things with injecting code and stuff.    |
+------------------------------------------------------------------*/
bool giveProcessDebugPrivilege() {
    HANDLE hToken = NULL;
    TOKEN_PRIVILEGES tokenPriv;
    LUID luidDebug;
    bool result = false;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
        if (LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidDebug)) {
            tokenPriv.PrivilegeCount           = 1;
            tokenPriv.Privileges[0].Luid       = luidDebug;
            tokenPriv.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            if (AdjustTokenPrivileges(hToken, FALSE, &tokenPriv, 0, NULL, NULL)) {
                Logger::info(TR("Successfully gave debug privilege to process"));
                result = true;
            }
            else {
                Logger::info(TR("Could not give debug privilege to process"));
                result = false;
            }
        }
    }
    CloseHandle(hToken);
    return result;
}

int main(int argc, char *argv[]) {
    // Ensure only one instance is running. If it's already running,
    // find that window and bring it to the front.
    // TODO: A better way is described at http://www.flounder.com/nomultiples.htm
    // I think the UNIQUE_TO_DESKTOP section of exclusion.cpp is what i need
    HANDLE mutex = CreateMutexA(0, true, "WD"APP_GUID);
    if (mutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);
        // TODO: find a good way of showing the existing window
        exit(0);
    }

    // Create the app instance and initialize
    WindowDetective app(argc, argv);

    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}