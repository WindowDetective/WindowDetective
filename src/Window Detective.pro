# This is the project file for Qt.
# It is used for the Qt pre-processor, qmake

TEMPLATE    = app
CONFIG      += windows uitools

# Output exe to release or debug folder, then copy to project's root dir
# This is so we don't get those .lib and .pdb files appearing in the root
# directory as well as the exe
# (Note: Hook project has a similar step configured)
CONFIG(release, debug|release) {
    DESTDIR = Release
} else {
    DESTDIR = Debug
}
QMAKE_POST_LINK += xcopy \"$${DESTDIR}\\$${TARGET}.exe\" \"$${DESTDIR}\\..\\..\" /Y

HEADERS      = window_detective/*.h           \
               inspector/*.h                  \
               inspector/ui_controls/*.h      \
               ui/*.h                         \
               ui/custom_widgets/*.h          \
               ui/property_pages/*.h

SOURCES      = window_detective/*.cpp         \
               inspector/*.cpp                \
               inspector/ui_controls/*.cpp    \
               ui/*.cpp                       \
               ui/custom_widgets/*.cpp        \
               ui/property_pages/*.cpp

FORMS        = forms/*.ui
UI_DIR       = forms

TRANSLATIONS = translations/el_GR.ts \
               translations/pl_PL.ts \
               translations/nl_NL.ts

# Setting UI_DIR seems to change the compiler's working directory, meaning
# that it can't find some header files. This seems to fix it.
INCLUDEPATH  += .\

RESOURCES    = qt_resources.qrc
RC_FILE      = win_resources.rc

LIBS         += -lkernel32 -luser32 -lShell32 -lAdvapi32 -lgdi32 -lPsapi

# Create linker address map for release. Helpfull when there are no debug symbols
QMAKE_LFLAGS_RELEASE += /MAP /MAPINFO:EXPORTS

# Link to release or debug hook DLL
CONFIG(release, debug|release) {
    LIBS += -l.\hook\Release\WD_Hook
} else {
    LIBS += -l.\hook\Debug\WD_Hook
}