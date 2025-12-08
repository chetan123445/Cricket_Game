@echo off
echo Compiling the project...
gcc -o cricket_game.exe accounts.c code.c history.c match.c teams.c tournament.c -I. -Wall -Wextra -std=c99
if %errorlevel% == 0 (
    echo Compilation successful. You can now run cricket_game.exe
) else (
    echo Compilation failed.
)
pause
