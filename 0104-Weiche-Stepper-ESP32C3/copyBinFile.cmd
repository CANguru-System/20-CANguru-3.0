@echo off
echo The source directory is: .\.pio\build\esp32c3_supermini\*.bin
echo The dest directory is: %CD%\binfiles\
copy .pio\build\esp32c3_supermini\*.bin binfiles\

pause
