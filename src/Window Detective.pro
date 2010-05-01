# This is the project file for Qt.
# It is used for the Qt pre-processor, qmake

TEMPLATE    = app
CONFIG      += windows uitools

# Output exe to release or debug folder, then copy to project's root dir
CONFIG(release, debug|release) {
    DESTDIR = Release
} else {
    DESTDIR = Debug
}
QMAKE_POST_LINK += xcopy \"$${DESTDIR}\\$${TARGET}.exe\" \"$${DESTDIR}\\..\\..\" /Y

HEADERS     = window_detective/*.h     \
              inspector/*.h            \
              ui/*.h                   \
              ui/custom_widgets/*.h

SOURCES     = window_detective/*.cpp   \
              inspector/*.cpp          \
              ui/*.cpp                 \
              ui/custom_widgets/*.cpp

FORMS       = forms/*.ui
UI_DIR      = forms

# Settings UI_DIR seems to change the compiler's working directory, meaning
# that it can't find some header files. This seems to fix it.
INCLUDEPATH += .\

RESOURCES   = qt_resources.qrc
RC_FILE     = win_resources.rc

LIBS        += -lkernel32 -luser32 -lShell32 -lAdvapi32 -lgdi32 -lPsapi

# Link to release or debug hook DLL
CONFIG(release, debug|release) {
    LIBS += -l.\hook\Release\WD_Hook
} else {
    LIBS += -l.\hook\Debug\WD_Hook
}