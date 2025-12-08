@echo off
echo Compiling the GUI project...

gcc accounts.c teams.c match.c tournament.c history.c ui.c gui.c -o gui.exe -I. -Llib -Wall -Wextra -std=c99 -lraylib -lopengl32 -lgdi32 -lwinmm

if %errorlevel% == 0 (
    echo Compilation successful. You can now run gui.exe
) else (
    echo Compilation failed.
)
pause