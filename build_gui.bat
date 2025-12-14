@echo off
echo Compiling the GUI project...

gcc src/accounts.c src/teams.c src/match.c src/tournament.c src/history.c src/ui.c src/gui.c src/field_setups.c "src/Play Match/toss.c" src/grounds.c -o gui.exe -I. -Isrc -Iinclude -Llib -Wall -Wextra -std=c99 -lraylib -lopengl32 -lgdi32 -lwinmm

if %errorlevel% == 0 (
    echo Compilation successful. You can now run gui.exe
) else (
    echo Compilation failed.
)
pause