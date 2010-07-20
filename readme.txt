

   --==  Window Detective 1.2.0 ==--
  =====================================


Window Detective is a programmer's tool used to "spy" on an application's
windows and allows you to view and even modify some of it's properties.


Features
--------
* View properties of windows such as text/title, dimensions and styles.
* Set properties such as text/title, dimensions and styles.
* View all windows in a tree hierarchy of parent/child windows.
* 'Pick' windows to inspect using the mouse.
* Find window/s by text, handle or class.
* Listen for window messages sent to a window.
* "Smart Settings" that intelligently remember user settings such as
  window positions.


Known Issues
------------
* Sometimes the hook DLL will not get immediately unloaded from certain processes.
  This seems to happen if that process does not handle any messages.
* When a lot of windows are destroyed, some of them may not be removed from the
  window tree. Refreshing the tree will fix this.


Feedback
--------
If you've got any question or suggestion or have any bugs to report, you can
create a tracker at https://sourceforge.net/tracker/?group_id=306398,
or drop by the forums: https://sourceforge.net/apps/phpbb/windowdetective.


Source Code
-----------
The source code for each version of Window Detective can be downloaded
from SourceForge at the same place as the installers.

It is packaged as a zip file which contains the source code as well as the
pre-compiled binaries and data required to run the program.
No installation is required to run, although settings will not be saved
unless the registry key exists in HKCU\Software.

To build the source code, you will need Microsoft Visual Studio 2008 and the
Qt framework to compile it. Qt is relatively easy to install and setup, you can
download version 4.6 from http://qt.nokia.com/downloads/windows-cpp-vs2008.
It may be possibly to build with other compilers although i haven't tried.
