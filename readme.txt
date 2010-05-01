

   --==  Window Detective 0.0.2 ==--

New Features:
 * Windows hooks monitor all windows for changes. Window Detective will update a window's
   properties when it's text, size, position, etc changed.

Known Bugs:
 * Sometimes crashes when too many windows are created/destroyed, or at other random
   times when window's update. This will only crash Window Detective, the DLL that is
   injected into other processes is quite stable and should not bring down the whole
   system :)


Window-Detective-0.0.1b-setup.exe
    This is the Window Installer package that will install Window Detective

Window-Detective-0.0.1b-src.zip
    This zip file contains the source code as well as the pre-compiled binary (exe).
    No installation is required to run, although settings will not be saved.

If you downloaded the source code, you will need Microsoft Visual Studio 2008 and the Qt
framework to compile it. Qt is relatively easy to install and setup, you can download
version 4.6 from http://qt.nokia.com/downloads/windows-cpp-vs2008
