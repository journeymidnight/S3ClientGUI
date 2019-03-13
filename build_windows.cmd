echo Set up environment...
set PATH=%QT%\bin\;C:\Qt\5.12\Tools\QtCreator\bin\;%PATH%

echo Building YourApp...
qmake -spec win32-msvc2015 CONFIG+=x86_64 CONFIG-=debug CONFIG+=release
nmake


