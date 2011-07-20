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


#include "main.h"
#include "inspector/WindowManager.h"
#include "inspector/MessageHandler.h"
#include "ui/MainWindow.h"
#include "ui/ActionManager.h"
#include "Settings.h"
#include "Logger.h"
#include "Shlobj.h"  // For getting user appdata path
using namespace inspector;

QCursor pickerCursor;
QPalette defaultPalette;   // So we can restore it if need be
String appPathString, userPathString;

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
    setWindowIcon(QIcon(":/Window Detective.ico"));

    QApplication::addLibraryPath(appPath());
    QApplication::setOrganizationName(APP_NAME);
    QApplication::setApplicationName(APP_NAME);

    Settings::read();
    Logger::initialize();
    giveProcessDebugPrivilege();
    InfoWindow::buildInfoLabels();
    loadPickerCursor();
    ActionManager::initialize();
    Resources::load(appPath()+"/data", userPath()+"/data");
    WindowManager::initialize();
    MessageHandler::initialize();
    SearchCriteria::initialize();
    setAppStyle(Settings::appStyle);
}

// Perform any aditional cleanup when the app quits
WindowDetective::~WindowDetective() {
    delete MessageHandler::current();
    delete WindowManager::current();
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
        Logger::osError("Could not load cursor. Using system crosshair cursor instead.");
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
| Returns the directory path where the application is installed.    |
| Separators are converted to use '/'.                              |
+------------------------------------------------------------------*/
String appPath() {
    if (appPathString.isEmpty()) {
        appPathString = QApplication::applicationDirPath();
        appPathString = QDir::fromNativeSeparators(appPathString);
    }
    return appPathString;
}

/*------------------------------------------------------------------+
| Returns the directory path of the user's application data.        |
| Separators are converted to use '/'.                              |
+------------------------------------------------------------------*/
String userPath() {
    if (userPathString.isEmpty()) {
        WCHAR szPath[MAX_PATH];

        HRESULT result = SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, szPath);
        if (FAILED(result)) {
            Logger::osError("Could not get application data folder.");
            return "";
        }

        userPathString = String::fromWCharArray(szPath);
        userPathString += "\\"APP_NAME;
        userPathString = QDir::fromNativeSeparators(userPathString);
    }
    return userPathString;
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
| Sets the application UI style using the selected built-in theme,  |
| and applies the Application.css style sheet. If the style is not  |
| native, the style's palette will be used instead of the system's. |
+------------------------------------------------------------------*/
void setAppStyle(String name) {
    static bool isFirstTime = true;

    if (name == "native") {
        // No need to reset native style if nothing else has been set yet
        if (!isFirstTime)
            restoreDefaultStyle();
    }
    else {
        QStyle* style = QStyleFactory::create(name);
        if (style) {
            QApplication::setPalette(style->standardPalette());
            QApplication::setStyle(style);
        }
        else {
            Logger::error(QObject::tr("Invalid application style: ")+name);
        }
    }

    String cssString;
    QTextStream stream(&cssString);
    loadCssStyle("Application", stream);
    qApp->setStyleSheet(cssString);

    isFirstTime = false;
}


/*------------------------------------------------------------------+
| Loads the application and user CSS files with the given name, and |
| writes them on the text stream in a <style> tag.                  |
| This function should really be in the UI package...               |
+------------------------------------------------------------------*/
void loadCssFile(String fileName, QTextStream& stream) {
    QFile file(fileName);
    if (file.exists() && file.open(QFile::ReadOnly)) {
        stream << file.readAll();
    }
}
void loadCssStyle(String name, QTextStream& stream) {
    loadCssFile(appPath()+"/styles/"+name+".css", stream);
    loadCssFile(userPath()+"/styles/"+name+".css", stream);
}

DWORD setProcessPrivileges(HANDLE hToken) {
    TOKEN_PRIVILEGES tp;
    TOKEN_PRIVILEGES tpPrev;
    LUID luidDebug;
    DWORD tpSize = sizeof(TOKEN_PRIVILEGES);

    ZeroMemory(&tpPrev, tpSize);
    
    if (!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidDebug)) {
		return GetLastError();
	}
	
	// First pass. Get current privilege setting
	tp.PrivilegeCount           = 1;
	tp.Privileges[0].Luid       = luidDebug;
	tp.Privileges[0].Attributes = 0;
    if (!AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), &tpPrev, &tpSize)) {
        return GetLastError();
    }
    
    // Second pass. Set privilege based on previous setting
    tpPrev.PrivilegeCount           = 1;
    tpPrev.Privileges[0].Luid       = luidDebug;
    tpPrev.Privileges[0].Attributes |= SE_PRIVILEGE_ENABLED;
    if (!AdjustTokenPrivileges(hToken, FALSE, &tpPrev, tpSize, NULL, NULL)) {
        return GetLastError();
    }
    
    return 0;
}

/*------------------------------------------------------------------+
| Attempts to give the current process debug privilege. With debug  |
| privilege we can do more things with injecting code and stuff.    |
+------------------------------------------------------------------*/
bool giveProcessDebugPrivilege() {
    HANDLE hToken = NULL;
    DWORD result = 0;
    
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        result = setProcessPrivileges(hToken);
    }
    else {
		result = GetLastError();
	}
	
    CloseHandle(hToken);
    
    if (result == 0) {
    	Logger::info(QObject::tr("Successfully gave debug privilege to process"));
    	return true;
	}
	else {
    	Logger::osError(result, QObject::tr("Could not give debug privilege to process"));
    	return false;
	}
}

int main(int argc, char *argv[]) {
    // Ensure only one instance is running. If it's already running,
    // find that window and bring it to the front.
    // TODO: A better way is described at http://www.flounder.com/nomultiples.htm
    // Also see http://doc.qt.nokia.com/solutions/4/qtsingleapplication/qtsingleapplication.html
    // It is exactly what i need and can even notify the other application to bring it's
    // window to the front (although i couldn't get that to work). But it uses the Qt network
    // module and so may be a bit bloated.
    // Download from http://qt.gitorious.org/qt-solutions
    HANDLE mutex = CreateMutexA(0, true, "WD"APP_GUID);
    if (mutex && GetLastError() == ERROR_ALREADY_EXISTS) {
        CloseHandle(mutex);
        // FIXME: No methods of bringing the window to the top seem to work
        HWND otherWindow = FindWindowA("QWidget", "Window Detective");
        if (otherWindow) {
            FlashWindow(otherWindow, FALSE);
        }
        exit(0);
    }

    // Create the app instance and initialize
    WindowDetective app(argc, argv);

    // If translation file exists, load it
    QTranslator translator;
    QDir dir(appPath(), "*.qm");
    QStringList tsFiles = dir.entryList();
    if (!tsFiles.isEmpty()) {
        translator.load(tsFiles.first(), appPath());
        app.installTranslator(&translator);
    }

    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}