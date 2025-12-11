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

## Field and Player Coordinates

This section provides a breakdown of the key coordinates and dimensions used for the cricket field in the game's GUI. This is useful for customizing fielding setups in `src/field_setups.c`.

All coordinates originate from a central point on the screen (`fieldCenter`), which is calculated dynamically based on screen resolution. The fielding setup coordinates in `src/field_setups.c` are **normalized**, meaning you should use values between -1.0 and 1.0 for the X and Y axes.

-   `{0, 0}` represents the absolute center of the pitch.
-   A positive Y value goes towards the bottom of the screen (towards the bowler's end).
-   A negative Y value goes towards the top of the screen (behind the batsman).
-   A positive X value goes to the right (off side for a right-handed batsman).
-   A negative X value goes to the left (leg side for a right-handed batsman).

Here are the specific measurements from the code:

### Field Dimensions

*   **Field Radius (`fieldRadius`):** The radius of the main green playing area. It is calculated dynamically based on your screen resolution and is the main scaling factor.
*   **Boundary Radius (`boundaryRadius`):** `fieldRadius + 15` pixels. The boundary rope is 15 pixels beyond the edge of the main field.
*   **30-Yard Circle Radius (`thirtyYardRadius`):** `fieldRadius * 0.45f`. The powerplay circle is 45% of the main field's radius.

### Pitch Dimensions

*   **Pitch Rectangle:** A rectangle of `400x50` pixels, centered horizontally and vertically on the `fieldCenter`.
    *   `x: fieldCenter.x - 200`
    *   `y: fieldCenter.y - 25`
    *   `width: 400`
    *   `height: 50`
*   **Creases:** Drawn at 20 pixels from the left edge of the pitch and 25 pixels from the right edge.

### Default Player Coordinates

These are the fixed starting positions for the main players, relative to the `fieldCenter`:

*   **Striker:** `{ fieldCenter.x + 180, fieldCenter.y }`
*   **Non-Striker:** `{ fieldCenter.x - 180, fieldCenter.y }`
*   **Wicket Keeper:** `{ fieldCenter.x + 220, fieldCenter.y }`
*   **Bowler (start of run-up):** `{ fieldCenter.x - 250, fieldCenter.y }`

### Fielding Positions

The fielder coordinates in `src/field_setups.c` are multiplied by the `fieldRadius` to place the fielders on the screen. The final calculation is:

```c
fielder_X = fieldCenter.x + (your_X_coordinate * (fieldRadius - 15));
fielder_Y = fieldCenter.y + (your_Y_coordinate * (fieldRadius - 15));
```

The `(fieldRadius - 15)` term is used to place fielders slightly inside the boundary edge.