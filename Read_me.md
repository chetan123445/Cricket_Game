# Cricket Tournament

This is a simple cricket tournament simulation game written in C. It allows you to create teams, manage players, and simulate matches and tournaments.

## Features

*   **User Authentication:** Register and login to manage your own profile and history.
*   **Team Management:** Create custom teams, add up to 22 players with different types (batsman, bowler, all-rounder, wicket-keeper) and skill ratings.
*   **Player Statistics:** View player statistics including runs scored and wickets taken.
*   **Match Simulation:** Simulate T20, ODI, and Test matches. The match simulation takes into account player skills to produce more realistic outcomes.
*   **Tournament Mode:** Create and run tournaments with multiple teams. The tournament follows a round-robin format, and a points table is displayed at the end.
*   **History:** Your match and tournament history is saved to your profile.

## Getting Started

To compile and run the project, you can use the provided `Makefile` or compile the source files manually with a C compiler like GCC.

### Prerequisites

*   A C compiler (e.g., GCC)
*   `make` (optional, but recommended)

### Compilation with Makefile

A `Makefile` is provided for easy compilation. Open a terminal in the project directory and run:

```sh
make
```

This will create an executable file named `cricket_game.exe`.

### Manual Compilation with GCC

If you don't have `make`, you can compile the project with GCC using the following command:

```sh
gcc -o cricket_game.exe accounts.c code.c history.c match.c teams.c tournament.c -I. -Wall -Wextra -std=c99
```

### Running the Game

Once compiled, you can run the game by executing the `cricket_game.exe` file:

```sh
./cricket_game.exe
```

## Project Structure

The project is organized into several modules, each with a specific responsibility:

*   `code.c`: The main entry point of the application. It contains the main menu and the primary application loop.
*   `accounts.c`/`.h`: Manages user registration and login.
*   `teams.c`/`.h`: Manages teams and players, including creating, listing, and viewing player statistics.
*   `match.c`/`.h`: Contains the match simulation logic.
*   `tournament.c`/`.h`: Manages tournaments, including creation, fixture generation, and gameplay.
*   `history.c`/`.h`: Manages user-specific history of matches and tournaments.
*   `common.h`: Contains common data structures and enumerations used throughout the project.
*   `Makefile`: The build script for the project.
*   `*.dat`: Data files used to store team, player, and user information.
