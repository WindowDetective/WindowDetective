///////////////////////////////////////////////////////////////////////////////
////////                                                                     //
//                                                                           //
//    W I N D O W                                                            //
//                                                                           //
//            D E T E C T I V E                                              //
//                                                                           //
//                                                                     ////////
///////////////////////////////////////////////////////////////////////////////


#include "main.h"
#include "inspector/WindowManager.h"
#include "inspector/MessageHandler.h"
#include "ui/MainWindow.h"
#include "Settings.h"
#include "Logger.h"
using namespace inspector;

QCursor pickerCursor;
QPalette defaultPalette;   // So we can restore it if need be

HMODULE KernelLibrary = NULL;
HMODULE PsApiLibrary = NULL;

/*------------------------------------------------------------------+
 | Main application constructor. Initializes other classes, loads   |
 | libraries and sets up various settings.                          |
 +------------------------------------------------------------------*/
WindowDetective::WindowDetective(int& argc, char** argv) :
    QApplication(argc, argv) {
    KernelLibrary = LoadLibrary(L"Kernel32.dll");
    PsApiLibrary = LoadLibrary(L"Psapi.dll");

    defaultPalette = QApplication::palette();
    setQuitOnLastWindowClosed(false);
    QDir::setCurrent(QApplication::applicationDirPath());
    QApplication::setOrganizationName(APP_NAME);
    QApplication::setApplicationName(APP_NAME);

    InfoWindow::buildInfoLabels();
    Settings::read();
    Logger::initialize();
    loadPickerCursor();
    WindowManager::initialize();
    MessageHandler::initialize();
    setAppStyle(Settings::appStyle);
}

// Perform any aditional cleanup when the app quits
WindowDetective::~WindowDetective() {
    FreeLibrary(KernelLibrary);
    FreeLibrary(PsApiLibrary);
}

/*------------------------------------------------------------------+
 | Loads the 'target' cursor from a .cur file. If there is an,      |
 | error the system crosshair cursor will be used instead.          |
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
 | Sets the cursor to the 'target' cursor for picking windows       |
 +------------------------------------------------------------------*/
void showPickerCursor() {
    QApplication::setOverrideCursor(pickerCursor);
}

void restoreCursor() {
    QApplication::restoreOverrideCursor();
}

/*------------------------------------------------------------------+
 | Since there seems to be no way of restoring the default theme,   |
 | we have to check the OS version to determine what theme the user |
 | was (hopefully) using.                                           |
 +------------------------------------------------------------------*/
void restoreDefaultStyle() {
    int osVersion = getOSVersion();
    QApplication::setPalette(defaultPalette);
    if (osVersion >= 600)
        QApplication::setStyle("windowsvista");
    else if (osVersion >= 501)
        QApplication::setStyle("windowsxp");
    else
        QApplication::setStyle("windows");
}

/*------------------------------------------------------------------+
 | Sets the application UI style using either an existing built-in  |
 | theme, or the style sheet in the "styles" folder. If the style   |
 | is not native, the style's palette will be used instead of the   |
 | system's.                                                        |
 +------------------------------------------------------------------*/
void setAppStyle(String name) {
    static bool isFirstTime = true;

    if (Settings::appStyle == "native") {
        // No need to reset native style if nothing else has been set yet
        if (!isFirstTime)
            restoreDefaultStyle();
    }
    else if (Settings::appStyle == "custom") {
        if (!isFirstTime)
            restoreDefaultStyle();
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