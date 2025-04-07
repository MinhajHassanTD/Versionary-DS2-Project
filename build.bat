@echo off
REM Set paths to libraries (adjust these to your actual installation paths)
set OPENCV_DIR=C:\opencv\build
set QT_DIR=C:\Qt\5.15.2\mingw81_64
set OPENSSL_DIR=C:\Program Files\OpenSSL-Win64

REM Create build directory if it doesn't exist
if not exist build mkdir build
if not exist build\bin mkdir build\bin

REM Compile source files
echo Compiling source files...
g++ -std=c++17 -c src\MerkleTree.cpp -o build\MerkleTree.o -I include -I %OPENCV_DIR%\include -I %OPENSSL_DIR%\include
g++ -std=c++17 -c src\Quadtree.cpp -o build\Quadtree.o -I include -I %OPENCV_DIR%\include
g++ -std=c++17 -c src\ImageProcessor.cpp -o build\ImageProcessor.o -I include -I %OPENCV_DIR%\include
g++ -std=c++17 -c src\Security.cpp -o build\Security.o -I include -I %OPENSSL_DIR%\include
g++ -std=c++17 -c src\VersionControl.cpp -o build\VersionControl.o -I include -I %OPENCV_DIR%\include -I %OPENSSL_DIR%\include
g++ -std=c++17 -c src\CLI.cpp -o build\CLI.o -I include -I %OPENCV_DIR%\include
g++ -std=c++17 -c src\GUI.cpp -o build\GUI.o -I include -I %OPENCV_DIR%\include -I %QT_DIR%\include -I %QT_DIR%\include\QtWidgets -I %QT_DIR%\include\QtGui -I %QT_DIR%\include\QtCore
g++ -std=c++17 -c src\main.cpp -o build\main.o -I include -I %OPENCV_DIR%\include -I %QT_DIR%\include -I %QT_DIR%\include\QtWidgets

REM Process Qt files (MOC)
echo Processing Qt files...
%QT_DIR%\bin\moc.exe include\GUI.h -o build\moc_GUI.cpp
g++ -std=c++17 -c build\moc_GUI.cpp -o build\moc_GUI.o -I include -I %QT_DIR%\include -I %QT_DIR%\include\QtWidgets -I %QT_DIR%\include\QtGui -I %QT_DIR%\include\QtCore

REM Link the executable
echo Linking executable...
g++ build\MerkleTree.o build\Quadtree.o build\ImageProcessor.o build\Security.o build\VersionControl.o build\CLI.o build\GUI.o build\moc_GUI.o build\main.o -o build\bin\versionary.exe -L %OPENCV_DIR%\x64\mingw\lib -L %QT_DIR%\lib -L %OPENSSL_DIR%\lib -lopencv_core455 -lopencv_imgproc455 -lopencv_imgcodecs455 -lopencv_highgui455 -lQt5Widgets -lQt5Gui -lQt5Core -llibssl -llibcrypto

REM Copy necessary DLLs
echo Copying DLLs...
copy %OPENCV_DIR%\x64\mingw\bin\*.dll build\bin\
copy %QT_DIR%\bin\Qt5Core.dll build\bin\
copy %QT_DIR%\bin\Qt5Gui.dll build\bin\
copy %QT_DIR%\bin\Qt5Widgets.dll build\bin\
copy %OPENSSL_DIR%\bin\libssl-1_1-x64.dll build\bin\
copy %OPENSSL_DIR%\bin\libcrypto-1_1-x64.dll build\bin\

echo Build completed.