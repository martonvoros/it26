@echo off
cd /d "%~dp0"
powershell -Command "Unblock-File -Path '%~dp0impulse-tracker.exe' -ErrorAction SilentlyContinue"
start "" "%~dp0impulse-tracker.exe" %*
