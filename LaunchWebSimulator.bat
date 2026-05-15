@echo off
setlocal

cd /d "%~dp0"

echo ============================================================
echo   PETS WEB SIMULATOR
echo ============================================================
echo.
echo Starting local static server on http://127.0.0.1:8000
echo.

start "PETS Web Server" cmd /k "cd /d %~dp0 && C:\Python314\python.exe -m http.server 8000 --bind 127.0.0.1"

timeout /t 2 /nobreak >nul
start "" http://127.0.0.1:8000/index.html

echo If the browser does not open, visit:
echo   http://127.0.0.1:8000/index.html
echo.
echo Close the "PETS Web Server" window to stop the site.
echo.
