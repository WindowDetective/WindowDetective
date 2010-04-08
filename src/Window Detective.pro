# This is the project file for Qt.
# It is used for the Qt pre-processor, qmake

TEMPLATE    = app
CONFIG      += windows uitools
DESTDIR     = ..

HEADERS     = window_detective/*.h     \
              inspector/*.h            \
              ui/*.h                   \
              ui/custom_widgets/*.h

SOURCES     = window_detective/*.cpp   \
              inspector/*.cpp          \
              ui/*.cpp                 \
              ui/custom_widgets/*.cpp

FORMS       = forms/*.ui

RESOURCES   = qt_resources.qrc
RC_FILE     = win_resources.rc

LIBS        += -lkernel32 -luser32 -lShell32 -lAdvapi32 -lgdi32 -lPsapi
