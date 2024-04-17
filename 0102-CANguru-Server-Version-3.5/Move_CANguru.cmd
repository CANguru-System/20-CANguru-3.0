@echo off
set source=CANguru\bin\Release\CANguru.exe
set dest=Files

echo.
echo CANguru - Helper fuer CANguru-Server
echo CANguru-Server wird in den Ordner Files kopiert
echo.
copy %source% %dest%

echo.
pause
