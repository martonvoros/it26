@echo off
cd /d "%~dp0"
powershell -Command "Unblock-File -Path '%~dp0IT26.exe' -ErrorAction SilentlyContinue"
start "" "%~dp0IT26.exe" %*
