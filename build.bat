@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvarsall.bat" amd64 >nul 2>&1

cd /d "C:\Users\da983\Desktop\GraphVisualizer\build"

"C:\Qt\Tools\CMake_64\bin\cmake.exe" .. -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH=C:/Qt/6.10.2/msvc2022_64 -DCMAKE_MAKE_PROGRAM=C:/Qt/Tools/Ninja/ninja.exe 2>&1
if errorlevel 1 (
    echo CMAKE_FAILED
    exit /b 1
)

"C:\Qt\Tools\Ninja\ninja.exe" -j4 2>&1
if errorlevel 1 (
    echo BUILD_FAILED
    exit /b 1
)

echo BUILD_SUCCESS
