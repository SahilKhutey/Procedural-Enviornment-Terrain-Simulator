@echo off
echo ============================================================
echo   PROCEDURAL ENVIRONMENTAL TERRAIN SIMULATOR (PETS) v1.0
echo ============================================================
echo.

:: --- ATTEMPT 1: Visual Studio 2022 ---
echo [SCAN] Checking for Visual Studio 2022...
if exist build rmdir /s /q build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% EQU 0 goto :COMPILE
cd ..

:: --- ATTEMPT 2: Visual Studio 2019 ---
echo [SCAN] Checking for Visual Studio 2019...
if exist build rmdir /s /q build
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019" -A x64 -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% EQU 0 goto :COMPILE
cd ..

:: --- ATTEMPT 3: MinGW (GCC) ---
echo [SCAN] Checking for MinGW/GCC...
if exist build rmdir /s /q build
mkdir build
cd build
cmake .. -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% EQU 0 goto :COMPILE
cd ..

:: --- ATTEMPT 4: Generic Fallback ---
echo [SCAN] Checking for Generic Default...
if exist build rmdir /s /q build
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% EQU 0 goto :COMPILE

echo.
echo [FATAL ERROR] NO VALID COMPILER FOUND.
echo.
echo [HOW TO FIX IN 2 MINUTES]:
echo 1. Open "Visual Studio Installer" from your Start Menu.
echo 2. Click "Modify" on your installation.
echo 3. Look for "Workloads" and check "Desktop development with C++".
echo 4. Click "Modify" (Bottom right) to install (~2-3 GB).
echo.
echo After this, the engine will compile and launch perfectly.
echo.
pause
exit

:COMPILE
echo.
echo [FOUND] Compiler recognized! Starting build...
cmake --build . --config Release --parallel 8

echo.
if exist Release\SimulationWorld.exe (
    echo [SUCCESS] Engine compiled successfully.
    start "" "Release\SimulationWorld.exe"
) else if exist SimulationWorld.exe (
    echo [SUCCESS] Engine compiled successfully.
    start "" "SimulationWorld.exe"
) else (
    echo [ERROR] Build failed during compilation. 
    pause
)
