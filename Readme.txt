Monte Carlo Raytracer Readme
----------------------------

Well not much to say yet.

Depends on QT, probably gonna depend on some more too, we'll see.

Done by Jan Schmid

------------------------------
How to prepare Qt 4.7.4 for VS2010
------------------------------
Copied unceremonosly from http://thomasstockx.blogspot.com/2011/03/qt-472-in-visual-studio-2010.html
thank you Thomas!

Install the Qt Visual Studio Add-In:
http://qt.nokia.com/downloads/visual-studio-add-in

Download the Qt source code as .zip from:
http://get.qt.nokia.com/qt/source/qt-everywhere-opensource-src-4.7.4.zip

Download and install the following patch for VS 2010:
http://support.microsoft.com/kb/2280741

Extract the thingy from step 4 to some decent location like C:\Qt\4.7.4\
Be sure the location has no spaces or special characters included.
After this step, C:\Qt\4.7.4\ should contain a lot of folders.

Add the folder from step 6 to your environment variables.
Windows 7: Control Panel –> System –> Advanced System Settings –> Environment Variables
Call it QTDIR.

Open a Visual Studio Command Prompt.

Make your way by cd’ing to the folder from step 6.

Run the following command:
”configure -debug-and-release -opensource -shared -no-qt3support -qt-sql-sqlite -phonon -phonon-backend -no-webkit -no-script -platform win32-msvc2010”

Explanation:
-debug-and-release: … if you don’t know what this means, please stop reading now.
-opensource: install open source edition of Qt.
-shared: makes the Qt libraries shared.
-no-qt3support: sorry my retro friends, don’t like old code.
-qt-sql-sqlite: enable support for the sqlite driver.
-phonon: compile the Phonon module (some multimedia thingy of Qt).
-phonon-backend: compile the platform-specific Phonon backend-plugin.
-no-webkit: disable WebKit module (heard of some bugs in combination with VS, just to be safe and since I don’t need it).
-no-script: same as no-webkit.
-platform win32-msvc2010: build for VS 2010!

Skip reading and press “y”.

Wait while Qt is getting configured for your platform.

When done, run “nmake” to start compiling Qt.

Wait again, long this time.
When it’s done (few warnings, no errors on my side) open Visual Studio 2010. Go to Qt –> Qt Options and add the folder from step 6.