#include "raylib.h"
#include "match.h"
#include "teams.h"
#include "accounts.h" // Include accounts for user types
#include <stdio.h> // For sprintf
#include <ctype.h> // For toupper()
#include <string.h>

// Define a simple structure to hold text box state
typedef struct {
    Rectangle bounds;
    char text[128];
    int charCount;
    bool active;
    bool isPassword;
} TextBox;

// Screen state management
typedef enum GameScreen { 
    SCREEN_LOGIN,
    SCREEN_REGISTER,
    SCREEN_MAIN_MENU,
    SCREEN_ADMIN_MENU,
    SCREEN_GAMEPLAY, 
    SCREEN_TEAMS, 
    SCREEN_UMPIRES,
    SCREEN_HISTORY,
    SCREEN_MANAGE_USERS
} GameScreen;

// Structure to hold all GUI state, including user info
typedef struct {
    GameScreen currentScreen;
    GameScreen previousScreen; // To enable a true "back" button
    UserType userType;
    char userEmail[128];
    char userName[64];
} GuiState;

// Forward declarations for screen functions
static void ChangeScreen(GuiState *state, GameScreen newScreen);
static void UpdateDrawLoginScreen(GuiState *state);
static void UpdateDrawRegisterScreen(GuiState *state);
static void UpdateDrawMainMenuScreen(GuiState *state);
static void UpdateDrawAdminMenuScreen(GuiState *state);
static void UpdateDrawGameplayScreen(GuiState *state, GameState *gameState);
static void UpdateDrawUmpiresScreen(GuiState *state);
static void UpdateDrawTeamsScreen(GuiState *state);
static void UpdateDrawPlaceholderScreen(GuiState *state, const char *title);
static void UpdateDrawManageUsersScreen(GuiState *state);

// Helper for text boxes
static void HandleTextBox(TextBox *box);

int main(void)
{
    // Set the window to be fullscreen on the primary monitor
    SetConfigFlags(FLAG_FULLSCREEN_MODE);
    const int screenWidth = GetMonitorWidth(GetCurrentMonitor());
    const int screenHeight = GetMonitorHeight(GetCurrentMonitor());

    InitWindow(screenWidth, screenHeight, "Cricket Management System");    
    
    // Match Game State Initialization
    GameState gameState = { 0 };
    Team teamA, teamB;
    
    initialize_dummy_teams(&teamA, &teamB);

    gameState.batting_team = &teamA;
    gameState.bowling_team = &teamB;
    gameState.max_overs = 20;

    // GUI State Initialization
    GuiState guiState = { 0 };
    guiState.currentScreen = SCREEN_LOGIN;
    guiState.previousScreen = SCREEN_LOGIN; // Initialize previous screen
    guiState.userType = USER_TYPE_NONE;

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())
    {
        switch(guiState.currentScreen) {
            case SCREEN_LOGIN:
                UpdateDrawLoginScreen(&guiState);
                break;
            case SCREEN_REGISTER:
                UpdateDrawRegisterScreen(&guiState);
                break;
            case SCREEN_MAIN_MENU:
                UpdateDrawMainMenuScreen(&guiState);
                break;
            case SCREEN_ADMIN_MENU:
                UpdateDrawAdminMenuScreen(&guiState);
                break;
            case SCREEN_GAMEPLAY:
                // We pass the gameState to the gameplay screen to be updated
                UpdateDrawGameplayScreen(&guiState, &gameState);
                break;
            case SCREEN_TEAMS:
                UpdateDrawTeamsScreen(&guiState);
                break;
            case SCREEN_UMPIRES:
                UpdateDrawUmpiresScreen(&guiState);
                break;
            case SCREEN_HISTORY:
                UpdateDrawPlaceholderScreen(&guiState, "Match History");
                break;
            case SCREEN_MANAGE_USERS:
                UpdateDrawManageUsersScreen(&guiState);
                break;
            default:
                break;
        }
    }

    CloseWindow();

    return 0;
}

// Helper function to change screens and track history
static void ChangeScreen(GuiState *state, GameScreen newScreen) {
    state->previousScreen = state->currentScreen;
    state->currentScreen = newScreen;
}

static void UpdateDrawLoginScreen(GuiState *state) {
    static TextBox emailBox;
    static TextBox passwordBox;
    static Rectangle loginButton;
    static Rectangle registerButton;
    static bool layoutInitialized = false;

    if (!layoutInitialized) {
        emailBox = (TextBox){ { GetScreenWidth()/2 - 200, GetScreenHeight()/2 - 100, 400, 50 }, {0}, 0, false, false };
        passwordBox = (TextBox){ { GetScreenWidth()/2 - 200, GetScreenHeight()/2 - 20, 400, 50 }, {0}, 0, false, true };
        loginButton = (Rectangle){ GetScreenWidth()/2 - 200, GetScreenHeight()/2 + 50, 400, 50 };
        registerButton = (Rectangle){ GetScreenWidth()/2 - 200, GetScreenHeight()/2 + 110, 400, 50 };
        layoutInitialized = true;
    }

    static const char *errorMessage = NULL;
    static bool isLoggingIn = false; // State to track login process

    HandleTextBox(&emailBox);
    HandleTextBox(&passwordBox);

    if (CheckCollisionPointRec(GetMousePosition(), loginButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isLoggingIn) {
        isLoggingIn = true;
        errorMessage = NULL;

        // --- BACKEND INTEGRATION POINT ---
        // Call the file-based login function from accounts.c with the GUI text box inputs.
        // This function should read from your users file without asking for console input.
        UserType type = login_user_from_file(emailBox.text, passwordBox.text, state->userName);
        
        // --- END INTEGRATION POINT ---

        if (type != USER_TYPE_NONE) {
            state->userType = type;
            strcpy(state->userEmail, emailBox.text); // Store email on successful login
            ChangeScreen(state, (type == USER_TYPE_ADMIN) ? SCREEN_ADMIN_MENU : SCREEN_MAIN_MENU);
            errorMessage = NULL;
        } else {
            errorMessage = "Login failed: Invalid credentials.";
        }

        isLoggingIn = false; // Reset login state after attempt
    }

    if (CheckCollisionPointRec(GetMousePosition(), registerButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && !isLoggingIn) {
        ChangeScreen(state, SCREEN_REGISTER);
        errorMessage = NULL;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Cricket Game Login", GetScreenWidth()/2 - MeasureText("Cricket Game Login", 50)/2, GetScreenHeight()/2 - 220, 50, DARKGRAY);

    DrawText("Email:", emailBox.bounds.x, emailBox.bounds.y - 20, 20, GRAY);
    DrawRectangleRec(emailBox.bounds, LIGHTGRAY);
    if (emailBox.active) DrawRectangleLinesEx(emailBox.bounds, 2, BLUE);
    DrawText(emailBox.text, emailBox.bounds.x + 5, emailBox.bounds.y + 10, 20, BLACK);

    DrawText("Password:", passwordBox.bounds.x, passwordBox.bounds.y - 20, 20, GRAY);
    DrawRectangleRec(passwordBox.bounds, LIGHTGRAY);
    if (passwordBox.active) DrawRectangleLinesEx(passwordBox.bounds, 2, BLUE);
    // Draw password as asterisks
    char passwordDisplay[128] = {0};
    for(int i = 0; i < passwordBox.charCount; ++i) strcat(passwordDisplay, "*");
    DrawText(passwordDisplay, passwordBox.bounds.x + 5, passwordBox.bounds.y + 10, 20, BLACK);

    // Draw Login Button with state-dependent text
    if (isLoggingIn) {
        DrawRectangleRec(loginButton, DARKGRAY);
        DrawText("Logging in...", loginButton.x + loginButton.width/2 - MeasureText("Logging in...", 20)/2, loginButton.y + 10, 20, WHITE);
    } else {
        DrawRectangleRec(loginButton, BLUE);
        DrawText("Login", loginButton.x + loginButton.width/2 - MeasureText("Login", 20)/2, loginButton.y + 10, 20, WHITE);
    }

    DrawRectangleRec(registerButton, GRAY);
    DrawText("Register New User", registerButton.x + registerButton.width/2 - MeasureText("Register New User", 20)/2, registerButton.y + 10, 20, WHITE);

    if (errorMessage) {
        DrawText(errorMessage, GetScreenWidth()/2 - MeasureText(errorMessage, 20)/2, GetScreenHeight()/2 + 180, 20, RED);
    }

    EndDrawing();
}

static void UpdateDrawRegisterScreen(GuiState *state) {
    // Similar structure to login screen, with more fields
    // For brevity, this is a placeholder. Implementation would mirror the login screen.
    const Rectangle backButton = { GetScreenWidth()/2 - 100, GetScreenHeight()/2 + 50, 200, 50 };
    if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, state->previousScreen); // Go back to the previous screen
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText("Registration", GetScreenWidth()/2 - MeasureText("Registration", 50)/2, GetScreenHeight()/2 - 100, 50, DARKGRAY);
    DrawText("Registration UI not yet implemented.", GetScreenWidth()/2 - MeasureText("Registration UI not yet implemented.", 20)/2, GetScreenHeight()/2 - 20, 20, GRAY);
    DrawRectangleRec(backButton, LIGHTGRAY);
    DrawText("Back to Login", backButton.x + backButton.width/2 - MeasureText("Back to Login", 20)/2, backButton.y + 10, 20, BLACK);
    EndDrawing();
}

static void UpdateDrawAdminMenuScreen(GuiState *state) {
    const int screenWidth = GetScreenWidth();
    const bool isSuper = is_superadmin(state->userEmail);
    const int buttonCount = isSuper ? 3 : 2; // Number of buttons changes based on user role
    const float totalHeight = buttonCount * 70 + (buttonCount - 1) * 10; // Total height of all buttons and spacing
    float startY = (GetScreenHeight() - totalHeight) / 2.0f;

    const Rectangle mainMenuButton = { screenWidth/2.0f - 200, startY, 400, 60 };
    const Rectangle manageUsersButton = { screenWidth/2.0f - 200, startY + 70, 400, 60 };
    const Rectangle teamsButton = { screenWidth/2.0f - 200, startY + (isSuper ? 140 : 70), 400, 60 };
    const Rectangle logoutButton = { screenWidth/2.0f - 200, startY + (isSuper ? 210 : 140), 400, 60 };

    if (CheckCollisionPointRec(GetMousePosition(), mainMenuButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, SCREEN_MAIN_MENU);
    }
    if (isSuper && CheckCollisionPointRec(GetMousePosition(), manageUsersButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, SCREEN_MANAGE_USERS);
    }
    if (CheckCollisionPointRec(GetMousePosition(), teamsButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, SCREEN_TEAMS);
    }
    if (CheckCollisionPointRec(GetMousePosition(), logoutButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        state->userType = USER_TYPE_NONE;
        memset(state->userEmail, 0, sizeof(state->userEmail));
        memset(state->userName, 0, sizeof(state->userName));
        ChangeScreen(state, SCREEN_LOGIN);
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    char title[128];
    sprintf(title, "Admin Menu: %s", state->userName);
    DrawText(title, screenWidth/2 - MeasureText(title, 50)/2, GetScreenHeight()/2 - 200, 50, DARKGRAY);

    DrawRectangleRec(mainMenuButton, LIGHTGRAY);
    DrawText("Main Admin Menu", mainMenuButton.x + mainMenuButton.width/2 - MeasureText("Main Admin Menu", 20)/2, mainMenuButton.y + 15, 20, BLACK);

    if (isSuper) {
        DrawRectangleRec(manageUsersButton, DARKBLUE);
        DrawText("Manage Users & Admins", manageUsersButton.x + manageUsersButton.width/2 - MeasureText("Manage Users & Admins", 20)/2, manageUsersButton.y + 15, 20, WHITE);
    }

    DrawRectangleRec(teamsButton, DARKBLUE);
    DrawText("Team & Player Management", teamsButton.x + teamsButton.width/2 - MeasureText("Team & Player Management", 20)/2, teamsButton.y + 15, 20, WHITE);

    DrawRectangleRec(logoutButton, MAROON);
    DrawText("Logout", logoutButton.x + logoutButton.width/2 - MeasureText("Logout", 20)/2, logoutButton.y + 15, 20, WHITE);
    EndDrawing();
}

static void UpdateDrawMainMenuScreen(GuiState *state) {
    const int screenWidth = GetScreenWidth();
    const Rectangle playButton = { screenWidth/2 - 200, GetScreenHeight()/2 - 180, 400, 60 };
    const Rectangle teamsButton = { screenWidth/2 - 200, GetScreenHeight()/2 - 110, 400, 60 };
    const Rectangle umpiresButton = { screenWidth/2 - 200, GetScreenHeight()/2 - 40, 400, 60 };
    const Rectangle historyButton = { screenWidth/2 - 200, GetScreenHeight()/2 + 30, 400, 60 };
    const Rectangle logoutButton = { screenWidth/2 - 200, GetScreenHeight()/2 + 100, 400, 60 };

    Vector2 mousePoint = GetMousePosition();

    if (CheckCollisionPointRec(mousePoint, playButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, SCREEN_GAMEPLAY);
    }
    if (CheckCollisionPointRec(mousePoint, teamsButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, SCREEN_TEAMS);
    }
    if (CheckCollisionPointRec(mousePoint, umpiresButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, SCREEN_UMPIRES);
    }
    if (CheckCollisionPointRec(mousePoint, historyButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, SCREEN_HISTORY);
    }
    if (CheckCollisionPointRec(mousePoint, logoutButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        state->userType = USER_TYPE_NONE;
        ChangeScreen(state, SCREEN_LOGIN);
    }


    BeginDrawing();
    ClearBackground(RAYWHITE);
    char title[128];
    sprintf(title, "Main Menu: %s", state->userName);
    DrawText(title, screenWidth/2 - MeasureText(title, 50)/2, GetScreenHeight()/2 - 280, 50, DARKGRAY);

    DrawRectangleRec(playButton, LIGHTGRAY);
    DrawText("Play Match", playButton.x + playButton.width/2 - MeasureText("Play Match", 20)/2, playButton.y + 15, 20, BLACK);

    DrawRectangleRec(teamsButton, LIGHTGRAY);
    DrawText("Teams & Players", teamsButton.x + teamsButton.width/2 - MeasureText("Teams & Players", 20)/2, teamsButton.y + 15, 20, BLACK);

    DrawRectangleRec(umpiresButton, LIGHTGRAY);
    DrawText("Add/View Umpires", umpiresButton.x + umpiresButton.width/2 - MeasureText("Add/View Umpires", 20)/2, umpiresButton.y + 15, 20, BLACK);

    DrawRectangleRec(historyButton, LIGHTGRAY);
    DrawText("View History", historyButton.x + historyButton.width/2 - MeasureText("View History", 20)/2, historyButton.y + 15, 20, BLACK);

    DrawRectangleRec(logoutButton, MAROON);
    DrawText("Logout", logoutButton.x + logoutButton.width/2 - MeasureText("Logout", 20)/2, logoutButton.y + 15, 20, WHITE);

    EndDrawing();
}

static void UpdateDrawGameplayScreen(GuiState *state, GameState *gameState) {
    const Rectangle backButton = { GetScreenWidth() - 170, GetScreenHeight() - 60, 150, 40 };

    if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Go back to the previous screen (likely the main menu)
        ChangeScreen(state, state->previousScreen);
    }

    static int game_over = 0;

    if (!game_over && IsKeyPressed(KEY_SPACE)) {
        if (gameState->overs_completed < gameState->max_overs && gameState->wickets < 10) {
            simulate_one_ball(gameState);
        } else {
            game_over = 1;
        }
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    char scoreText[100];
    sprintf(scoreText, "%s: %d / %d", gameState->batting_team->name, gameState->total_runs, gameState->wickets);
    DrawText(scoreText, 20, 20, 40, DARKGREEN);

    char oversText[100];
    sprintf(oversText, "Overs: %d.%d", gameState->overs_completed, gameState->balls_bowled_in_over);
    DrawText(oversText, 20, 70, 20, BLACK);
    
    Player *striker = &gameState->batting_team->players[gameState->striker_idx];
    Player *bowler = &gameState->bowling_team->players[gameState->bowler_idx];

    char strikerText[100], bowlerText[100];
    sprintf(strikerText, "Striker: %s", striker->name);
    DrawText(strikerText, 20, 120, 20, BLACK);
    sprintf(bowlerText, "Bowler: %s", bowler->name);
    DrawText(bowlerText, 20, 150, 20, BLACK);
    
    if (!game_over) {
        DrawText("Press [SPACE] to bowl the next ball", 20, GetScreenHeight() - 40, 20, DARKGRAY);
    } else {
        DrawText("MATCH FINISHED", 200, 200, 50, RED);
    }

    // Draw the back button
    DrawRectangleRec(backButton, LIGHTGRAY);
    DrawText("Back to Menu", backButton.x + backButton.width/2 - MeasureText("Back to Menu", 20)/2, backButton.y + 10, 20, BLACK);

    EndDrawing();
}

// Case-insensitive string search (strstr)
static const char* stristr(const char* haystack, const char* needle) {
    if (!*needle) return haystack;
    for (; *haystack; ++haystack) {
        if (toupper((unsigned char)*haystack) == toupper((unsigned char)*needle)) {
            const char* h, *n;
            for (h = haystack, n = needle; *h && *n; ++h, ++n) {
                if (toupper((unsigned char)*h) != toupper((unsigned char)*n)) break;
            }
            if (!*n) return haystack;
        }
    }
    return NULL;
}

// Helper function to draw text with a bold effect
static void DrawTextBold(const char *text, int posX, int posY, int fontSize, Color color) {
    DrawText(text, posX + 1, posY + 1, fontSize, color); // Draw a slight shadow to create a bold effect
    DrawText(text, posX, posY, fontSize, color);
}

static void UpdateDrawUmpiresScreen(GuiState *state) {
    // --- State for this screen ---
    static Umpire *umpires = NULL;
    static int num_umpires = 0;
    static bool needs_refresh = true;
    static Vector2 scroll = { 0.0f, 0.0f };
    static int editIndex = -1; // -1 means not editing, >= 0 is the index of the umpire being edited

    // Input boxes for adding a new umpire
    static TextBox nameBox = { { 20, 100, 200, 30 }, {0}, 0, false, false };
    static TextBox countryBox = { { 230, 100, 150, 30 }, {0}, 0, false, false };
    static TextBox yearBox = { { 390, 100, 100, 30 }, {0}, 0, false, false };
    static TextBox matchesBox = { { 500, 100, 100, 30 }, {0}, 0, false, false };
    const Rectangle addButton = { 610, 100, 170, 30 };
    const Rectangle backButton = { 20, GetScreenHeight() - 50, 150, 40 };
    const Rectangle cancelEditButton = { 610, 135, 170, 25 };

    // Search and Filter boxes
    static TextBox searchBox = { { 20, 60, 200, 25 }, {0}, 0, false, false };
    static TextBox filterCountryBox;
    static TextBox filterMatchesBox;
    static TextBox filterYearBox;
    static bool filterBoxesInitialized = false;

    if (!filterBoxesInitialized) {
        filterCountryBox = (TextBox){ { GetScreenWidth() - 430, 60, 100, 25 }, {0}, 0, false, false };
        filterMatchesBox = (TextBox){ { GetScreenWidth() - 320, 60, 100, 25 }, {0}, 0, false, false };
        filterYearBox = (TextBox){ { GetScreenWidth() - 210, 60, 100, 25 }, {0}, 0, false, false };
        filterBoxesInitialized = true;
    }
    
    const Rectangle clearFiltersButton = { GetScreenWidth() - 100, 60, 80, 25 };


    // --- Logic ---
    if (needs_refresh) {
        if (umpires) free(umpires);
        umpires = load_umpires(&num_umpires);
        needs_refresh = false;
    }

    HandleTextBox(&nameBox);
    HandleTextBox(&countryBox);
    HandleTextBox(&yearBox);
    HandleTextBox(&matchesBox);
    HandleTextBox(&searchBox);
    HandleTextBox(&filterCountryBox);
    HandleTextBox(&filterMatchesBox);
    HandleTextBox(&filterYearBox);

    // Clear text boxes helper lambda
    auto void clear_boxes() {
        memset(&nameBox.text, 0, sizeof(nameBox.text)); nameBox.charCount = 0;
        memset(&countryBox.text, 0, sizeof(countryBox.text)); countryBox.charCount = 0;
        memset(&yearBox.text, 0, sizeof(yearBox.text)); yearBox.charCount = 0;
        memset(&matchesBox.text, 0, sizeof(matchesBox.text)); matchesBox.charCount = 0;
    };

    if (CheckCollisionPointRec(GetMousePosition(), clearFiltersButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        memset(&searchBox.text, 0, sizeof(searchBox.text)); searchBox.charCount = 0;
        memset(&filterCountryBox.text, 0, sizeof(filterCountryBox.text)); filterCountryBox.charCount = 0;
        memset(&filterMatchesBox.text, 0, sizeof(filterMatchesBox.text)); filterMatchesBox.charCount = 0;
        memset(&filterYearBox.text, 0, sizeof(filterYearBox.text)); filterYearBox.charCount = 0;
    }


    if (CheckCollisionPointRec(GetMousePosition(), addButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (nameBox.charCount > 0 && countryBox.charCount > 0 && yearBox.charCount > 0 && matchesBox.charCount > 0) {
            if (editIndex == -1) { // ADD NEW UMPIRE
                num_umpires++;
                umpires = realloc(umpires, num_umpires * sizeof(Umpire));
                Umpire *new_ump = &umpires[num_umpires - 1];
                strcpy(new_ump->name, nameBox.text);
                strcpy(new_ump->country, countryBox.text);
                new_ump->since_year = atoi(yearBox.text);
                new_ump->matches_umpired = atoi(matchesBox.text);
            } else { // SAVE EDITED UMPIRE
                Umpire *edited_ump = &umpires[editIndex];
                strcpy(edited_ump->name, nameBox.text);
                strcpy(edited_ump->country, countryBox.text);
                edited_ump->since_year = atoi(yearBox.text);
                edited_ump->matches_umpired = atoi(matchesBox.text);
                editIndex = -1; // Exit edit mode
            }

            save_umpires(umpires, num_umpires);
            clear_boxes();
            needs_refresh = true; // Trigger a reload to be safe
        }
    }

    // Handle Cancel Edit button
    if (editIndex != -1 && CheckCollisionPointRec(GetMousePosition(), cancelEditButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        editIndex = -1;
        clear_boxes();
    }

    if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, state->previousScreen);
        needs_refresh = true; // Ensure it refreshes next time we visit
    }

    // --- Drawing ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    DrawTextBold("Manage Umpires", GetScreenWidth()/2 - MeasureText("Manage Umpires", 40)/2, 10, 40, DARKGRAY);
    
    // --- Draw Search and Filter UI ---
    DrawTextBold("Search Name:", searchBox.bounds.x, searchBox.bounds.y - 15, 10, GRAY);
    DrawRectangleRec(searchBox.bounds, WHITE); DrawRectangleLinesEx(searchBox.bounds, 1, LIGHTGRAY);
    DrawTextBold(searchBox.text, searchBox.bounds.x + 5, searchBox.bounds.y + 5, 20, BLACK);

    DrawTextBold("Filter Country:", filterCountryBox.bounds.x, filterCountryBox.bounds.y - 15, 10, GRAY);
    DrawRectangleRec(filterCountryBox.bounds, WHITE); DrawRectangleLinesEx(filterCountryBox.bounds, 1, LIGHTGRAY);
    DrawTextBold(filterCountryBox.text, filterCountryBox.bounds.x + 5, filterCountryBox.bounds.y + 5, 20, BLACK);

    DrawTextBold("Matches >=", filterMatchesBox.bounds.x, filterMatchesBox.bounds.y - 15, 10, GRAY);
    DrawRectangleRec(filterMatchesBox.bounds, WHITE); DrawRectangleLinesEx(filterMatchesBox.bounds, 1, LIGHTGRAY);
    DrawTextBold(filterMatchesBox.text, filterMatchesBox.bounds.x + 5, filterMatchesBox.bounds.y + 5, 20, BLACK);

    DrawTextBold("Year >=", filterYearBox.bounds.x, filterYearBox.bounds.y - 15, 10, GRAY);
    DrawRectangleRec(filterYearBox.bounds, WHITE); DrawRectangleLinesEx(filterYearBox.bounds, 1, LIGHTGRAY);
    DrawTextBold(filterYearBox.text, filterYearBox.bounds.x + 5, filterYearBox.bounds.y + 5, 20, BLACK);

    DrawRectangleRec(clearFiltersButton, LIGHTGRAY);
    DrawTextBold("Clear", clearFiltersButton.x + clearFiltersButton.width/2 - MeasureText("Clear", 10)/2, clearFiltersButton.y + 8, 10, BLACK);

    // --- Filtering Logic ---
    Umpire* filtered_umpires = NULL;
    int num_filtered_umpires = 0;
    int* original_indices = NULL;

    if (num_umpires > 0) {
        filtered_umpires = malloc(num_umpires * sizeof(Umpire));
        original_indices = malloc(num_umpires * sizeof(int));
        int min_matches = atoi(filterMatchesBox.text);
        int min_year = atoi(filterYearBox.text);

        for (int i = 0; i < num_umpires; i++) {
            bool pass = true;
            if (searchBox.charCount > 0 && !stristr(umpires[i].name, searchBox.text)) pass = false;
            if (filterCountryBox.charCount > 0 && !stristr(umpires[i].country, filterCountryBox.text)) pass = false;
            if (min_matches > 0 && umpires[i].matches_umpired < min_matches) pass = false;
            if (min_year > 0 && umpires[i].since_year < min_year) pass = false;

            if (pass) {
                filtered_umpires[num_filtered_umpires] = umpires[i];
                original_indices[num_filtered_umpires] = i;
                num_filtered_umpires++;
            }
        }
    }

    // Draw input section
    DrawTextBold("Name", nameBox.bounds.x, nameBox.bounds.y - 20, 10, GRAY);
    DrawRectangleRec(nameBox.bounds, LIGHTGRAY); DrawTextBold(nameBox.text, nameBox.bounds.x + 5, nameBox.bounds.y + 8, 20, BLACK);
    DrawTextBold("Country", countryBox.bounds.x, countryBox.bounds.y - 20, 10, GRAY);
    DrawRectangleRec(countryBox.bounds, LIGHTGRAY); DrawTextBold(countryBox.text, countryBox.bounds.x + 5, countryBox.bounds.y + 8, 20, BLACK);
    DrawTextBold("Since (Year)", yearBox.bounds.x, yearBox.bounds.y - 20, 10, GRAY);
    DrawRectangleRec(yearBox.bounds, LIGHTGRAY); DrawTextBold(yearBox.text, yearBox.bounds.x + 5, yearBox.bounds.y + 8, 20, BLACK);
    DrawTextBold("Matches", matchesBox.bounds.x, matchesBox.bounds.y - 20, 10, GRAY);
    DrawRectangleRec(matchesBox.bounds, LIGHTGRAY); DrawTextBold(matchesBox.text, matchesBox.bounds.x + 5, matchesBox.bounds.y + 8, 20, BLACK);

    // Draw "Add" or "Save" button based on edit mode
    const char* buttonText = (editIndex == -1) ? "Add Umpire" : "Save Changes";
    DrawRectangleRec(addButton, (editIndex == -1) ? DARKGREEN : BLUE);
    DrawTextBold(buttonText, addButton.x + addButton.width/2 - MeasureText(buttonText, 20)/2, addButton.y + 5, 20, WHITE);
    if (editIndex != -1) {
        DrawRectangleRec(cancelEditButton, MAROON);
        DrawTextBold("Cancel", cancelEditButton.x + cancelEditButton.width/2 - MeasureText("Cancel", 15)/2, cancelEditButton.y + 5, 15, WHITE);
    }

    // Draw list of current umpires
    DrawRectangle(20, 150, GetScreenWidth() - 40, GetScreenHeight() - 210, DARKBROWN);
    DrawTextBold("Current Umpires", 30, 160, 20, GOLD);
    DrawLine(30, 185, GetScreenWidth() - 50, 185, GOLD);
    
    // Define the scrollable view area
    Rectangle view = { 20, 188, GetScreenWidth() - 40, GetScreenHeight() - 250 }; // The visible panel
    const float itemHeight = 25.0f;
    const float headerHeight = 40.0f;
    // Calculate the total height of the content
    const float contentHeight = (num_filtered_umpires * itemHeight) + headerHeight;

    // Handle mouse wheel scrolling if content is larger than view
    if (contentHeight > view.height && CheckCollisionPointRec(GetMousePosition(), view)) {
        scroll.y += GetMouseWheelMove() * itemHeight; // Scroll one item at a time
        if (scroll.y > 0) scroll.y = 0; // Clamp top
        if (scroll.y < view.height - contentHeight) scroll.y = view.height - contentHeight; // Clamp bottom
    }

    // Start clipping the drawing to the view rectangle
    BeginScissorMode(view.x, view.y, view.width, view.height);

    if (num_filtered_umpires > 0) {
        const float editButtonWidth = 60;
        const float deleteButtonWidth = 70;
        const float buttonHeight = 20;
        // --- DYNAMIC COLUMN WIDTH CALCULATION ---
        // 1. Find the maximum width for each column to ensure perfect alignment
        float max_name_width = MeasureText("Name", 20);
        float max_country_width = MeasureText("Country", 20);

        for (int i = 0; i < num_filtered_umpires; i++) {
            float name_width = MeasureText(filtered_umpires[i].name, 20);
            if (name_width > max_name_width) max_name_width = name_width;
            float country_width = MeasureText(filtered_umpires[i].country, 20);
            if (country_width > max_country_width) max_country_width = country_width;
        }

        // 2. Define column start positions with padding
        const int padding = 25;
        float col1_x = 40;
        float col2_x = col1_x + max_name_width + padding;
        float col3_x = col2_x + max_country_width + padding;
        float col4_x = col3_x + MeasureText("Since: YYYY", 20) + padding; // Approx width
        float col5_x = col4_x + MeasureText("Matches: 999", 20) + padding;
        
        // 3. Draw the header (it scrolls with the content)
        int header_y = view.y + 5 + scroll.y;
        DrawTextBold("Name", col1_x, header_y, 20, GOLD);
        DrawTextBold("Country", col2_x, header_y, 20, GOLD);
        DrawTextBold("Since", col3_x, header_y, 20, GOLD);
        DrawTextBold("Matches", col4_x, header_y, 20, GOLD);
        DrawTextBold("Actions", col5_x, header_y, 20, GOLD);
        DrawLine(view.x + 10, header_y + 25, view.x + view.width - 20, header_y + 25, GOLD);

        // 4. Draw each umpire's details in perfectly aligned columns
        for (int i = 0; i < num_filtered_umpires; i++) {
            int y_pos = view.y + headerHeight + (i * itemHeight) + scroll.y;
            int original_index = original_indices[i];
            Color textColor = (editIndex == original_index) ? YELLOW : RAYWHITE; // Highlight the entry being edited

            DrawTextBold(filtered_umpires[i].name, col1_x, y_pos, 20, textColor);
            DrawTextBold(filtered_umpires[i].country, col2_x, y_pos, 20, textColor);
            DrawTextBold(TextFormat("%d", filtered_umpires[i].since_year), col3_x, y_pos, 20, RAYWHITE);
            DrawTextBold(TextFormat("%d", filtered_umpires[i].matches_umpired), col4_x, y_pos, 20, RAYWHITE);

            // Define and draw Edit/Delete buttons for this row
            Rectangle editBtnRec = { col5_x, y_pos, editButtonWidth, buttonHeight };
            Rectangle deleteBtnRec = { col5_x + editButtonWidth + 5, y_pos, deleteButtonWidth, buttonHeight };

            DrawRectangleRec(editBtnRec, ORANGE);
            DrawTextBold("Edit", editBtnRec.x + editBtnRec.width/2 - MeasureText("Edit", 15)/2, editBtnRec.y + 2, 15, BLACK);
            DrawRectangleRec(deleteBtnRec, RED);
            DrawTextBold("Delete", deleteBtnRec.x + deleteBtnRec.width/2 - MeasureText("Delete", 15)/2, deleteBtnRec.y + 2, 15, WHITE);

            // Check for button clicks
            if (CheckCollisionPointRec(GetMousePosition(), editBtnRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                editIndex = original_index;
                // Load data into text boxes
                strcpy(nameBox.text, umpires[original_index].name); nameBox.charCount = strlen(nameBox.text);
                strcpy(countryBox.text, umpires[original_index].country); countryBox.charCount = strlen(countryBox.text);
                sprintf(yearBox.text, "%d", umpires[original_index].since_year); yearBox.charCount = strlen(yearBox.text);
                sprintf(matchesBox.text, "%d", umpires[original_index].matches_umpired); matchesBox.charCount = strlen(matchesBox.text);
            }

            if (CheckCollisionPointRec(GetMousePosition(), deleteBtnRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Shift all subsequent elements up
                for (int j = original_index; j < num_umpires - 1; j++) {
                    umpires[j] = umpires[j + 1];
                }
                num_umpires--;
                if (num_umpires > 0) umpires = realloc(umpires, num_umpires * sizeof(Umpire)); else { free(umpires); umpires = NULL; }
                save_umpires(umpires, num_umpires);
                needs_refresh = true;
                break; // Exit loop as the array has been modified
            }
        }
    } else {
        DrawTextBold("No umpires found in umpires.dat.", view.x + 20, view.y + 10, 20, GRAY); // No scroll needed
    }

    // Stop clipping
    EndScissorMode();

    // Draw the scrollbar if needed
    if (contentHeight > view.height) {
        Rectangle scrollBarArea = { view.x + view.width - 12, view.y, 10, view.height };
        DrawRectangleRec(scrollBarArea, LIGHTGRAY);
        
        float scrollBarHeight = (view.height / contentHeight) * view.height;
        float scrollBarY = view.y + (-scroll.y / (contentHeight - view.height)) * (view.height - scrollBarHeight);
        Rectangle scrollHandle = { scrollBarArea.x, scrollBarY, 10, scrollBarHeight };
        DrawRectangleRec(scrollHandle, DARKGRAY);
    }

    if (filtered_umpires) free(filtered_umpires);
    if (original_indices) free(original_indices);

    // Back button
    DrawRectangleRec(backButton, LIGHTGRAY);
    DrawTextBold("Back to Menu", backButton.x + backButton.width/2 - MeasureText("Back to Menu", 20)/2, backButton.y + 10, 20, BLACK);

    EndDrawing();
}

static void UpdateDrawTeamsScreen(GuiState *state) {
    // --- Screen State ---
    static Team* teams = NULL;
    static int num_teams = 0;
    static bool needs_refresh = true;
    static int selectedTeamIndex = -1;
    static int editTeamIndex = -1; // -1 for adding, >=0 for editing

    // --- UI element definitions ---
    const Rectangle panelLeft = { 10, 10, (GetScreenWidth() / 3.5f) - 15, GetScreenHeight() - 20 };
    const Rectangle panelRight = { panelLeft.x + panelLeft.width + 10, 10, GetScreenWidth() - panelLeft.width - 30, GetScreenHeight() - 20 };
    const Rectangle backButton = { panelLeft.x + 10, panelLeft.y + panelLeft.height - 50, panelLeft.width - 20, 40 };

    // --- Left Panel (Teams List) ---
    static TextBox newTeamBox = { {0}, {0}, 0, false, false };
    static Rectangle addTeamButton;
    static Vector2 teamScroll = { 0, 0 }; // For scrolling through teams

    // --- Right Panel (Players List & Add Player Form) ---
    static bool showAddPlayerForm = false;
    static Player new_player_template; // Holds data for the new player being created
    static TextBox addPlayerNameBox = { {0}, {0}, 0, false, false };
    static TextBox batSkillBox, bowlSkillBox, fieldSkillBox;
    static TextBox matchesBox, runsBox, wicketsBox, stumpsBox, runOutsBox;
    static Rectangle savePlayerButton, cancelPlayerButton;
    static Vector2 playerScroll = { 0, 0 }; // For scrolling through players
    static int editPlayerIndex = -1; // -1 for adding, >=0 for editing a player
    static Vector2 formScroll = { 0, 0 }; // For scrolling the add player form

    const char* playerTypeNames[] = { "Batsman", "Bowler", "All-Rounder", "Wicketkeeper" };
    const char* batStyleNames[] = { "Right-Handed", "Left-Handed" };
    const char* bowlStyleNames[] = { "None", "RF(Right Arm Fast)", "LF(Left Arm Fast)", "RFM(Right Arm Medium Fast)", "LFM(Left Arm Medium Fast)", "RM(Right Arm Medium)", "LM(Left Arm Medium)", "Right Arm Off-Spin", "Right Arm Leg-Spin", "Left Arm Orthodox Spin", "Left Arm Wrist Spin(Chinaman)" };

    // --- Initialization and Layout ---
    static bool layoutInitialized = false;
    if (!layoutInitialized) {
        newTeamBox.bounds = (Rectangle){ panelLeft.x + 10, panelLeft.y + 50, panelLeft.width - 70, 30 };
        addTeamButton = (Rectangle){ newTeamBox.bounds.x + newTeamBox.bounds.width + 5, newTeamBox.bounds.y, 45, 30 };
        
        // Layout for the detailed add player form
        float formX = panelRight.x + 20;
        float formY = panelRight.y + 100;
        addPlayerNameBox.bounds = (Rectangle){ formX, formY, 250, 30 };
        batSkillBox.bounds = (Rectangle){ formX, formY + 200, 80, 30 };
        bowlSkillBox.bounds = (Rectangle){ formX + 90, formY + 200, 80, 30 };
        fieldSkillBox.bounds = (Rectangle){ formX + 180, formY + 200, 80, 30 };
        matchesBox.bounds = (Rectangle){ formX, formY + 270, 80, 30 };
        runsBox.bounds = (Rectangle){ formX + 90, formY + 270, 80, 30 };
        wicketsBox.bounds = (Rectangle){ formX + 180, formY + 270, 80, 30 };
        stumpsBox.bounds = (Rectangle){ formX, formY + 340, 80, 30 };
        runOutsBox.bounds = (Rectangle){ formX + 90, formY + 340, 80, 30 };
        
        layoutInitialized = true;
    }

    // --- Logic ---
    if (needs_refresh) {
        if (teams) free(teams);
        teams = load_teams(&num_teams);
        needs_refresh = false;
    }

    HandleTextBox(&newTeamBox);
    if (showAddPlayerForm) {
        HandleTextBox(&addPlayerNameBox);
        HandleTextBox(&batSkillBox);
        HandleTextBox(&bowlSkillBox);
        HandleTextBox(&fieldSkillBox);
        HandleTextBox(&matchesBox);
        HandleTextBox(&runsBox);
        HandleTextBox(&wicketsBox);
        HandleTextBox(&stumpsBox);
        HandleTextBox(&runOutsBox);
    }
    
    // --- Team Management Logic ---
    // Add a new team
    if (CheckCollisionPointRec(GetMousePosition(), addTeamButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (newTeamBox.charCount > 0) { // Ensure name is not empty
            if (editTeamIndex == -1) { // Add mode
                num_teams++;
                teams = realloc(teams, num_teams * sizeof(Team));
                Team *new_team = &teams[num_teams - 1];
                strcpy(new_team->name, newTeamBox.text);
                new_team->num_players = 0;
                new_team->is_hidden = false;
                new_team->is_deleted = false;
            } else { // Edit mode
                // IMPORTANT: When renaming a team, we must save all teams to update players.dat correctly.
                strcpy(teams[editTeamIndex].name, newTeamBox.text);
                editTeamIndex = -1; // Exit edit mode
            }
            save_teams(teams, num_teams);
            memset(newTeamBox.text, 0, sizeof(newTeamBox.text));
            newTeamBox.charCount = 0;
            needs_refresh = true;
        }
    }

    // --- Player Management Logic ---
    // Save the newly created player
    savePlayerButton = (Rectangle){ panelRight.x + panelRight.width - 320, panelRight.y + panelRight.height - 50, 150, 40 };
    cancelPlayerButton = (Rectangle){ panelRight.x + panelRight.width - 160, panelRight.y + panelRight.height - 50, 150, 40 };

    if (showAddPlayerForm && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), savePlayerButton)) {
        if (addPlayerNameBox.charCount > 0 && selectedTeamIndex != -1) {
            Team* selectedTeam = &teams[selectedTeamIndex];
            if (selectedTeam->num_players < MAX_PLAYERS) {
                Player* p;
                if (editPlayerIndex == -1) {
                    // Adding a new player
                    p = &selectedTeam->players[selectedTeam->num_players++];
                } else {
                    // Updating an existing player
                    p = &selectedTeam->players[editPlayerIndex];
                }
                *p = new_player_template; // Copy from the form template
                strcpy(p->name, addPlayerNameBox.text); // Copy name from text box
                // Copy numerical values from text boxes
                p->batting_skill = atoi(batSkillBox.text); p->bowling_skill = atoi(bowlSkillBox.text); p->fielding_skill = atoi(fieldSkillBox.text);
                p->matches_played = atoi(matchesBox.text); p->total_runs = atoi(runsBox.text); p->total_wickets = atoi(wicketsBox.text);
                p->total_stumpings = atoi(stumpsBox.text); p->total_run_outs = atoi(runOutsBox.text);
                
                save_teams(teams, num_teams);
                needs_refresh = true;
                showAddPlayerForm = false; // Hide form after saving
                editPlayerIndex = -1; // Reset edit mode
            }
        }
    }

    // Back button logic
    if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, state->previousScreen);
        needs_refresh = true;
    }
    
    // Cancel adding a player
    if (showAddPlayerForm && IsMouseButtonPressed(MOUSE_LEFT_BUTTON) && CheckCollisionPointRec(GetMousePosition(), cancelPlayerButton)) {
        editPlayerIndex = -1;
        showAddPlayerForm = false;
    }

    // --- Drawing ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    // Draw Left Panel (Teams)
    DrawRectangleRec(panelLeft, (Color){ 240, 240, 240, 255 });
    DrawRectangleLinesEx(panelLeft, 1, DARKGRAY);
    DrawTextBold("Teams", panelLeft.x + 10, panelLeft.y + 15, 20, DARKGRAY);
    DrawRectangleRec(newTeamBox.bounds, WHITE);
    DrawRectangleLinesEx(newTeamBox.bounds, 1, editTeamIndex != -1 ? BLUE : GRAY);
    DrawText(newTeamBox.text, newTeamBox.bounds.x + 5, newTeamBox.bounds.y + 5, 20, BLACK);
    
    const char* teamButtonText = (editTeamIndex == -1) ? "Add" : "Save";
    DrawRectangleRec(addTeamButton, (editTeamIndex == -1) ? DARKGREEN : BLUE);
    DrawText(teamButtonText, addTeamButton.x + 5, addTeamButton.y + 5, 20, WHITE);

    // Scrollable area for teams
    Rectangle teamListView = { panelLeft.x + 1, newTeamBox.bounds.y + newTeamBox.bounds.height + 10, panelLeft.width - 2, panelLeft.height - newTeamBox.bounds.height - 70 };
    // TODO: Implement scrolling for team list if it gets long
    DrawRectangleRec(teamListView, WHITE);
    
    // Team list logic
    for (int i = 0; i < num_teams; i++) {
        Rectangle itemRec = { teamListView.x + 5, teamListView.y + 5 + (i * 30) + teamScroll.y, teamListView.width - 175, 25 };
        Rectangle editBtn = { itemRec.x + itemRec.width + 5, itemRec.y, 50, 25 };
        Rectangle deleteBtn = { editBtn.x + editBtn.width + 5, itemRec.y, 50, 25 };
        Rectangle hideBtn = { deleteBtn.x + deleteBtn.width + 5, itemRec.y, 50, 25 };

        if (CheckCollisionPointRec(GetMousePosition(), itemRec) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (editTeamIndex != -1) editTeamIndex = -1; // Cancel edit if selecting another team
            selectedTeamIndex = i;
            showAddPlayerForm = false; // Always hide form when selecting a new team
        }
        Color bgColor = (selectedTeamIndex == i) ? SKYBLUE : (teams[i].is_deleted ? DARKGRAY : LIGHTGRAY);
        Color textColor = teams[i].is_deleted ? GRAY : BLACK;

        DrawRectangleRec(itemRec, bgColor);
        char teamNameDisplay[MAX_TEAM_NAME_LEN + 24] = {0};
        strcpy(teamNameDisplay, teams[i].name);
        if (teams[i].is_deleted) strcat(teamNameDisplay, " [Deleted]");
        if (teams[i].is_hidden && !teams[i].is_deleted) strcat(teamNameDisplay, " [Hidden]");
        DrawText(teamNameDisplay, itemRec.x + 5, itemRec.y + 5, 20, textColor);

        // --- Draw Conditional Buttons: Edit/Delete for active teams, Recover for deleted teams ---
        if (teams[i].is_deleted) {
            // Draw only a "Recover" button
            Rectangle recoverBtn = { editBtn.x, editBtn.y, 105, 25 };
            DrawRectangleRec(recoverBtn, DARKGREEN);
            DrawText("Recover", recoverBtn.x + 20, recoverBtn.y + 5, 15, WHITE);
            if (CheckCollisionPointRec(GetMousePosition(), recoverBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                teams[i].is_deleted = false;
                save_teams(teams, num_teams);
                needs_refresh = true;
            }
        } else {
            // Draw Edit/Delete/Hide buttons for active teams
            DrawRectangleRec(editBtn, ORANGE); DrawText("Edit", editBtn.x + 10, editBtn.y + 5, 15, BLACK);
            DrawRectangleRec(deleteBtn, RED); DrawText("Del", deleteBtn.x + 10, deleteBtn.y + 5, 15, WHITE);

            const char* hideText = teams[i].is_hidden ? "Show" : "Hide";
            DrawRectangleRec(hideBtn, teams[i].is_hidden ? SKYBLUE : VIOLET);
            DrawText(hideText, hideBtn.x + 10, hideBtn.y + 5, 15, WHITE);

            if (CheckCollisionPointRec(GetMousePosition(), editBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                editTeamIndex = i;
                strcpy(newTeamBox.text, teams[i].name); newTeamBox.charCount = strlen(newTeamBox.text);
            }
            if (CheckCollisionPointRec(GetMousePosition(), deleteBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                teams[i].is_deleted = true;
                save_teams(teams, num_teams); needs_refresh = true;
            }
            if (CheckCollisionPointRec(GetMousePosition(), hideBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                teams[i].is_hidden = !teams[i].is_hidden;
                save_teams(teams, num_teams); needs_refresh = true;
            }
        }
    }

    // Draw Right Panel (Players)
    DrawRectangleRec(panelRight, (Color){ 250, 250, 250, 255 });
    DrawRectangleLinesEx(panelRight, 1, DARKGRAY);

    if (showAddPlayerForm) {
        // --- DRAW THE DETAILED ADD PLAYER FORM ---
        const char* formTitle = (editPlayerIndex == -1) ? "Create New Player" : "Edit Player";
        DrawTextBold(formTitle, panelRight.x + 10, panelRight.y + 15, 20, DARKBLUE);
        
        // Form content area
        Rectangle formView = { panelRight.x + 1, panelRight.y + 50, panelRight.width - 2, panelRight.height - 110 };
        float contentHeight = 500; // Estimated height of the form

        // --- Define a structured, two-column layout ---
        const float ROW_HEIGHT = 55.0f;
        const float COL1_X = formView.x + 20;
        const float COL2_X = formView.x + (formView.width / 2.0f);
        float currentY = formView.y + 10 + formScroll.y;

        if (CheckCollisionPointRec(GetMousePosition(), formView)) {
            formScroll.y += GetMouseWheelMove() * 25.0f;
            if (formScroll.y > 0) formScroll.y = 0;
            if (formScroll.y < formView.height - contentHeight) formScroll.y = formView.height - contentHeight;
        }

        BeginScissorMode(formView.x, formView.y, formView.width, formView.height);
        // --- Column 1: Primary Details ---
        DrawRectangle(COL1_X - 10, currentY, (formView.width / 2.0f) - 20, 30, DARKGRAY);
        DrawTextBold("Primary Details", COL1_X, currentY + 5, 20, WHITE);
        currentY += 40;

        // Player Name
        DrawTextBold("Player Name:", COL1_X, currentY - 15, 10, GRAY);
        addPlayerNameBox.bounds = (Rectangle){ COL1_X, currentY, (formView.width / 2.0f) - 40, 35 }; // This was line 1206
        DrawRectangleRec(addPlayerNameBox.bounds, WHITE); DrawRectangleLinesEx(addPlayerNameBox.bounds, 1, GRAY);
        DrawText(addPlayerNameBox.text, addPlayerNameBox.bounds.x + 10, addPlayerNameBox.bounds.y + 8, 20, BLACK);
        currentY += ROW_HEIGHT;

        // Player Type
        DrawTextBold("Player Role:", COL1_X, currentY - 15, 10, GRAY);
        for (int i = 0; i < 4; i++) {
            Rectangle typeBtn = { COL1_X + (i * 120), currentY, 110, 35 };
            bool isSelected = (new_player_template.type == (PlayerType)i);
            DrawRectangleRec(typeBtn, isSelected ? DARKBLUE : LIGHTGRAY);
            DrawText(playerTypeNames[i], typeBtn.x + 10, typeBtn.y + 8, 20, isSelected ? WHITE : BLACK);
            if (CheckCollisionPointRec(GetMousePosition(), typeBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) new_player_template.type = (PlayerType)i;
        }
        currentY += ROW_HEIGHT;

        // Batting and Bowling Style
        DrawTextBold("Batting Style:", COL1_X, currentY - 15, 10, GRAY);
        Rectangle batStyleBtn = { COL1_X, currentY, 230, 35 };
        DrawRectangleRec(batStyleBtn, LIGHTGRAY);
        DrawText(batStyleNames[new_player_template.batting_style], batStyleBtn.x + 10, batStyleBtn.y + 8, 20, BLACK);
        if (CheckCollisionPointRec(GetMousePosition(), batStyleBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) new_player_template.batting_style = (BattingStyle)!new_player_template.batting_style;

        DrawTextBold("Bowling Style:", COL1_X + 250, currentY - 15, 10, GRAY);
        Rectangle bowlStyleBtn = { COL1_X + 250, currentY, 230, 35 };
        DrawRectangleRec(bowlStyleBtn, LIGHTGRAY);
        DrawText(bowlStyleNames[new_player_template.bowling_style], bowlStyleBtn.x + 10, bowlStyleBtn.y + 8, 20, BLACK);
        if (CheckCollisionPointRec(GetMousePosition(), bowlStyleBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            new_player_template.bowling_style = (BowlingStyle)((new_player_template.bowling_style + 1) % (sizeof(bowlStyleNames)/sizeof(bowlStyleNames[0])));
        }
        currentY += ROW_HEIGHT;

        // Status (Wicketkeeper, Active)
        Rectangle wkCheck = { COL1_X, currentY, 25, 25 };
        DrawRectangleRec(wkCheck, LIGHTGRAY);
        if (new_player_template.is_wicketkeeper) DrawRectangle(wkCheck.x + 5, wkCheck.y + 5, 15, 15, DARKBLUE);
        DrawTextBold("Is Wicketkeeper?", wkCheck.x + 35, wkCheck.y + 5, 20, BLACK);
        if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){wkCheck.x, wkCheck.y, 200, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) new_player_template.is_wicketkeeper = !new_player_template.is_wicketkeeper;

        Rectangle activeCheck = { COL1_X + 250, currentY, 25, 25 };
        DrawRectangleRec(activeCheck, LIGHTGRAY);
        if (new_player_template.is_active) DrawRectangle(activeCheck.x + 5, activeCheck.y + 5, 15, 15, DARKGREEN);
        DrawTextBold("Is Active?", activeCheck.x + 35, activeCheck.y + 5, 20, BLACK);
        if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){activeCheck.x, activeCheck.y, 150, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) new_player_template.is_active = !new_player_template.is_active;
        currentY += ROW_HEIGHT;

        // --- Column 2: Skills and Stats ---
        currentY = formView.y + 10 + formScroll.y; // Reset Y for the second column
        DrawRectangle(COL2_X - 10, currentY, (formView.width / 2.0f) - 20, 30, DARKGRAY);
        DrawTextBold("Skills & Career Stats", COL2_X, currentY + 5, 20, WHITE);
        currentY += 40;

        // Skills
        DrawTextBold("Skills (1-100)", COL2_X, currentY - 15, 10, GRAY);
        batSkillBox.bounds = (Rectangle){ COL2_X, currentY, 100, 35 };
        bowlSkillBox.bounds = (Rectangle){ COL2_X + 120, currentY, 100, 35 };
        fieldSkillBox.bounds = (Rectangle){ COL2_X + 240, currentY, 100, 35 };
        DrawText("Bat", batSkillBox.bounds.x, batSkillBox.bounds.y + 37, 10, GRAY);
        DrawRectangleRec(batSkillBox.bounds, WHITE); DrawText(batSkillBox.text, batSkillBox.bounds.x + 10, batSkillBox.bounds.y + 8, 20, BLACK);
        DrawText("Bowl", bowlSkillBox.bounds.x, bowlSkillBox.bounds.y + 37, 10, GRAY);
        DrawRectangleRec(bowlSkillBox.bounds, WHITE); DrawText(bowlSkillBox.text, bowlSkillBox.bounds.x + 10, bowlSkillBox.bounds.y + 8, 20, BLACK);
        DrawText("Field", fieldSkillBox.bounds.x, fieldSkillBox.bounds.y + 37, 10, GRAY);
        DrawRectangleRec(fieldSkillBox.bounds, WHITE); DrawText(fieldSkillBox.text, fieldSkillBox.bounds.x + 10, fieldSkillBox.bounds.y + 8, 20, BLACK);
        currentY += ROW_HEIGHT;

        // Career Stats
        DrawTextBold("Career Stats (Initial Values)", COL2_X, currentY - 15, 10, GRAY);
        matchesBox.bounds = (Rectangle){ COL2_X, currentY, 100, 35 };
        runsBox.bounds = (Rectangle){ COL2_X + 120, currentY, 100, 35 };
        wicketsBox.bounds = (Rectangle){ COL2_X + 240, currentY, 100, 35 };
        stumpsBox.bounds = (Rectangle){ COL2_X, currentY + 45, 100, 35 };
        runOutsBox.bounds = (Rectangle){ COL2_X + 120, currentY + 45, 100, 35 };
        DrawText("Matches", matchesBox.bounds.x, matchesBox.bounds.y + 37, 10, GRAY);
        DrawRectangleRec(matchesBox.bounds, WHITE); DrawText(matchesBox.text, matchesBox.bounds.x + 10, matchesBox.bounds.y + 8, 20, BLACK);
        DrawText("Runs", runsBox.bounds.x, runsBox.bounds.y + 37, 10, GRAY);
        DrawRectangleRec(runsBox.bounds, WHITE); DrawText(runsBox.text, runsBox.bounds.x + 10, runsBox.bounds.y + 8, 20, BLACK);
        DrawText("Wickets", wicketsBox.bounds.x, wicketsBox.bounds.y + 37, 10, GRAY);
        DrawRectangleRec(wicketsBox.bounds, WHITE); DrawText(wicketsBox.text, wicketsBox.bounds.x + 10, wicketsBox.bounds.y + 8, 20, BLACK);
        DrawText("Stumps", stumpsBox.bounds.x, stumpsBox.bounds.y + 37, 10, GRAY);
        DrawRectangleRec(stumpsBox.bounds, WHITE); DrawText(stumpsBox.text, stumpsBox.bounds.x + 10, stumpsBox.bounds.y + 8, 20, BLACK);
        DrawText("RunOuts", runOutsBox.bounds.x, runOutsBox.bounds.y + 37, 10, GRAY);
        DrawRectangleRec(runOutsBox.bounds, WHITE); DrawText(runOutsBox.text, runOutsBox.bounds.x + 10, runOutsBox.bounds.y + 8, 20, BLACK);

        EndScissorMode();

        // Draw Save/Cancel buttons for the form
        const char* saveButtonText = (editPlayerIndex == -1) ? "Save Player" : "Save Changes";
        DrawRectangleRec(savePlayerButton, (editPlayerIndex == -1) ? DARKGREEN : BLUE);
        DrawTextBold(saveButtonText, savePlayerButton.x + 20, savePlayerButton.y + 10, 20, WHITE);
        DrawRectangleRec(cancelPlayerButton, MAROON);
        DrawTextBold("Cancel", cancelPlayerButton.x + 40, cancelPlayerButton.y + 10, 20, WHITE);


    } else if (selectedTeamIndex != -1) {
        if (teams[selectedTeamIndex].is_deleted) {
            DrawTextBold("This team has been deleted.", panelRight.x + 10, panelRight.y + 15, 20, MAROON);
        } else {
            // --- DRAW THE PLAYER LIST ---
        char panelTitle[128];
        sprintf(panelTitle, "Players for %s", teams[selectedTeamIndex].name);
        DrawTextBold(panelTitle, panelRight.x + 10, panelRight.y + 15, 20, DARKBLUE);

        // Button to show the add player form
        Rectangle openAddPlayerFormButton = { panelRight.x + panelRight.width - 220, panelRight.y + 10, 200, 30 };
        DrawRectangleRec(openAddPlayerFormButton, DARKGREEN);
        DrawTextBold("Add New Player", openAddPlayerFormButton.x + 20, openAddPlayerFormButton.y + 5, 20, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), openAddPlayerFormButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            showAddPlayerForm = true;
            editPlayerIndex = -1; // Ensure we are in "add" mode
            // Reset the template to defaults
            memset(&new_player_template, 0, sizeof(Player));
            new_player_template.batting_skill = 70; new_player_template.bowling_skill = 70; new_player_template.fielding_skill = 70;
            new_player_template.is_active = true;
            // Reset text boxes
            memset(&addPlayerNameBox.text, 0, sizeof(addPlayerNameBox.text)); addPlayerNameBox.charCount = 0;
            sprintf(batSkillBox.text, "70"); batSkillBox.charCount = 2;
            sprintf(bowlSkillBox.text, "70"); bowlSkillBox.charCount = 2;
            sprintf(fieldSkillBox.text, "70"); fieldSkillBox.charCount = 2;
            sprintf(matchesBox.text, "0"); matchesBox.charCount = 1; sprintf(runsBox.text, "0"); runsBox.charCount = 1;
            sprintf(wicketsBox.text, "0"); wicketsBox.charCount = 1; sprintf(stumpsBox.text, "0"); stumpsBox.charCount = 1;
            sprintf(runOutsBox.text, "0"); runOutsBox.charCount = 1;
        }

        // Player List
        Rectangle playerListView = { panelRight.x + 1, panelRight.y + 50, panelRight.width - 2, panelRight.height - 60 };
        DrawRectangleRec(playerListView, WHITE);

        // Header for player list
        // Handle horizontal scrolling for the player list
        if (CheckCollisionPointRec(GetMousePosition(), playerListView)) {
            playerScroll.y += GetMouseWheelMove() * 25.0f;
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                playerScroll.x += GetMouseWheelMove() * 25.0f;
            }
        }

        BeginScissorMode(playerListView.x, playerListView.y, playerListView.width, playerListView.height);

        // --- Column Definitions and Sizing ---
        const int col_padding = 15;
        float col_widths[] = { 150, 120, 50, 80, 40, 70, 80, 80, 80, 40, 70, 70, 80, 80, 120 }; // Last is for actions
        const char* col_headers[] = { "Name", "Role", "Bat", "Bowl", "WK", "Active", "Bat Skl", "Bwl Skl", "Fld Skl", "M", "Runs", "Wkts", "Stumps", "RunOuts", "Actions" };
        float col_x[15];
        col_x[0] = playerListView.x + 10 + playerScroll.x;
        for (int i = 1; i < 15; i++) {
            col_x[i] = col_x[i-1] + col_widths[i-1] + col_padding;
        }

        // Draw Headers
        for (int i = 0; i < 15; i++) {
            DrawTextBold(col_headers[i], col_x[i], playerListView.y + 10, 20, DARKGRAY);
        }

        Team* selectedTeam = &teams[selectedTeamIndex];
        for (int i = 0; i < selectedTeam->num_players; i++) {
            Player* p = &selectedTeam->players[i];
            float y_pos = playerListView.y + 40 + (i * 25) + playerScroll.y;
            
            // Draw all player details in their respective columns, perfectly aligned
            DrawText(p->name, col_x[0], y_pos, 20, BLACK);
            DrawText(playerTypeNames[p->type], col_x[1], y_pos, 20, DARKGRAY);
            DrawText(p->batting_style == BATTING_STYLE_RHB ? "RHB" : "LHB", col_x[2], y_pos, 20, DARKGRAY);
            DrawText(bowlStyleNames[p->bowling_style], col_x[3], y_pos, 20, DARKGRAY);
            DrawText(p->is_wicketkeeper ? "Yes" : "No", col_x[4], y_pos, 20, DARKGRAY);
            DrawText(p->is_active ? "Yes" : "No", col_x[5], y_pos, 20, DARKGRAY);
            DrawText(TextFormat("%d", p->batting_skill), col_x[6], y_pos, 20, BLUE);
            DrawText(TextFormat("%d", p->bowling_skill), col_x[7], y_pos, 20, BLUE);
            DrawText(TextFormat("%d", p->fielding_skill), col_x[8], y_pos, 20, BLUE);
            DrawText(TextFormat("%d", p->matches_played), col_x[9], y_pos, 20, DARKPURPLE);
            DrawText(TextFormat("%d", p->total_runs), col_x[10], y_pos, 20, DARKPURPLE);
            DrawText(TextFormat("%d", p->total_wickets), col_x[11], y_pos, 20, DARKPURPLE);
            DrawText(TextFormat("%d", p->total_stumpings), col_x[12], y_pos, 20, DARKPURPLE);
            DrawText(TextFormat("%d", p->total_run_outs), col_x[13], y_pos, 20, DARKPURPLE);

            // Draw Edit/Delete buttons for the player
            Rectangle editBtn = { col_x[14], y_pos, 50, 20 };
            Rectangle deleteBtn = { col_x[14] + 55, y_pos, 60, 20 };
            DrawRectangleRec(editBtn, ORANGE); DrawText("Edit", editBtn.x + 10, editBtn.y + 2, 15, BLACK);
            DrawRectangleRec(deleteBtn, RED); DrawText("Delete", deleteBtn.x + 5, deleteBtn.y + 2, 15, WHITE);

            if (CheckCollisionPointRec(GetMousePosition(), editBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                editPlayerIndex = i;
                showAddPlayerForm = true;
                // Load player data into the form template and text boxes
                new_player_template = *p;
                strcpy(addPlayerNameBox.text, p->name); addPlayerNameBox.charCount = strlen(p->name);
                sprintf(batSkillBox.text, "%d", p->batting_skill); batSkillBox.charCount = strlen(batSkillBox.text);
                sprintf(bowlSkillBox.text, "%d", p->bowling_skill); bowlSkillBox.charCount = strlen(bowlSkillBox.text);
                sprintf(fieldSkillBox.text, "%d", p->fielding_skill); fieldSkillBox.charCount = strlen(fieldSkillBox.text);
                sprintf(matchesBox.text, "%d", p->matches_played); matchesBox.charCount = strlen(matchesBox.text);
                sprintf(runsBox.text, "%d", p->total_runs); runsBox.charCount = strlen(runsBox.text);
                sprintf(wicketsBox.text, "%d", p->total_wickets); wicketsBox.charCount = strlen(wicketsBox.text);
                sprintf(stumpsBox.text, "%d", p->total_stumpings); stumpsBox.charCount = strlen(stumpsBox.text);
                sprintf(runOutsBox.text, "%d", p->total_run_outs); runOutsBox.charCount = strlen(runOutsBox.text);
            }

            if (CheckCollisionPointRec(GetMousePosition(), deleteBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Shift all subsequent players up to delete the current one
                for (int j = i; j < selectedTeam->num_players - 1; j++) {
                    selectedTeam->players[j] = selectedTeam->players[j + 1];
                }
                selectedTeam->num_players--;
                save_teams(teams, num_teams);
                needs_refresh = true;
                break; // Exit loop as the player array has been modified
            }
        }
        
        EndScissorMode();
        }

    } else {
        DrawTextBold("Select a team to view players", panelRight.x + 10, panelRight.y + 15, 20, GRAY);
    }

    // Draw Back Button
    DrawRectangleRec(backButton, MAROON);
    DrawTextBold("Back", backButton.x + backButton.width/2 - MeasureText("Back", 20)/2, backButton.y + 10, 20, WHITE);

    EndDrawing();
}

static void UpdateDrawPlaceholderScreen(GuiState *state, const char *title) {
    const Rectangle backButton = { 20, GetScreenHeight() - 60, 150, 40 };

    if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Go back to the previous screen, which is the correct menu
        ChangeScreen(state, state->previousScreen);
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawText(title, GetScreenWidth()/2 - MeasureText(title, 40)/2, 150, 40, DARKGRAY);
    DrawRectangleRec(backButton, LIGHTGRAY);
    DrawText("Back to Menu", backButton.x + backButton.width/2 - MeasureText("Back to Menu", 20)/2, backButton.y + 10, 20, BLACK);
    EndDrawing();
}

static void UpdateDrawManageUsersScreen(GuiState *state) {
    // --- State for this screen ---
    static Account *accounts = NULL;
    static int num_accounts = 0;
    static bool needs_refresh = true;
    static Vector2 scroll = { 0.0f, 0.0f };
    static int editIndex = -1; // -1 for adding, >= 0 for editing

    // --- UI element definitions ---
    static TextBox nameBox, emailBox, passwordBox;
    static bool isAdminFlag = false;
    static Rectangle addButton;
    const Rectangle backButton = { 20, GetScreenHeight() - 50, 150, 40 };

    static bool layoutInitialized = false;
    if (!layoutInitialized) {
        nameBox = (TextBox){ { 20, 100, 200, 30 }, {0}, 0, false, false };
        emailBox = (TextBox){ { 230, 100, 250, 30 }, {0}, 0, false, false };
        passwordBox = (TextBox){ { 490, 100, 200, 30 }, {0}, 0, false, true };
        addButton = (Rectangle){ 820, 100, 170, 30 };
        layoutInitialized = true;
    }

    // --- Logic ---
    if (needs_refresh) {
        if (accounts) free(accounts);
        accounts = load_all_accounts(&num_accounts);
        needs_refresh = false;
    }

    HandleTextBox(&nameBox);
    HandleTextBox(&emailBox);
    HandleTextBox(&passwordBox);

    auto void clear_boxes() {
        memset(&nameBox.text, 0, sizeof(nameBox.text)); nameBox.charCount = 0;
        memset(&emailBox.text, 0, sizeof(emailBox.text)); emailBox.charCount = 0;
        memset(&passwordBox.text, 0, sizeof(passwordBox.text)); passwordBox.charCount = 0;
        isAdminFlag = false;
    };

    // Handle Add/Save button
    if (CheckCollisionPointRec(GetMousePosition(), addButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (nameBox.charCount > 0 && emailBox.charCount > 0) {
            if (editIndex == -1) { // ADD NEW
                bool exists = false;
                for(int i=0; i<num_accounts; ++i) if(strcmp(accounts[i].email, emailBox.text) == 0) exists = true;

                if (!exists && passwordBox.charCount > 0) {
                    num_accounts++;
                    accounts = realloc(accounts, num_accounts * sizeof(Account));
                    Account *new_acc = &accounts[num_accounts - 1];
                    strcpy(new_acc->name, nameBox.text);
                    strcpy(new_acc->email, emailBox.text);
                    strcpy(new_acc->password, passwordBox.text);
                    new_acc->isAdmin = isAdminFlag;
                }
            } else { // SAVE EDIT
                Account *acc = &accounts[editIndex];
                if (!is_superadmin(acc->email)) { // Superadmin cannot be edited
                    strcpy(acc->name, nameBox.text);
                    if (passwordBox.charCount > 0) strcpy(acc->password, passwordBox.text);
                    acc->isAdmin = isAdminFlag;
                }
                editIndex = -1;
            }
            save_all_accounts(accounts, num_accounts);
            clear_boxes();
            needs_refresh = true;
        }
    }

    // Handle Back button
    if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, SCREEN_ADMIN_MENU);
        needs_refresh = true; // Force refresh when leaving
    }

    // --- Drawing ---
    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawTextBold("Manage Users & Admins", GetScreenWidth()/2 - MeasureText("Manage Users & Admins", 40)/2, 10, 40, DARKGRAY);

    // --- Draw Input Form ---
    DrawTextBold("Name", nameBox.bounds.x, nameBox.bounds.y - 20, 10, GRAY);
    DrawRectangleRec(nameBox.bounds, LIGHTGRAY); DrawText(nameBox.text, nameBox.bounds.x + 5, nameBox.bounds.y + 8, 20, BLACK);

    DrawTextBold("Email", emailBox.bounds.x, emailBox.bounds.y - 20, 10, GRAY);
    DrawRectangleRec(emailBox.bounds, LIGHTGRAY);
    if (editIndex != -1) DrawRectangleRec(emailBox.bounds, (Color){220,220,220,255}); // Gray out email on edit
    DrawText(emailBox.text, emailBox.bounds.x + 5, emailBox.bounds.y + 8, 20, BLACK);

    DrawTextBold(editIndex == -1 ? "Password" : "New Password (optional)", passwordBox.bounds.x, passwordBox.bounds.y - 20, 10, GRAY);
    DrawRectangleRec(passwordBox.bounds, LIGHTGRAY);
    char pass_display[128] = {0}; for(int i=0; i<passwordBox.charCount; ++i) strcat(pass_display, "*");
    DrawText(pass_display, passwordBox.bounds.x + 5, passwordBox.bounds.y + 8, 20, BLACK);

    Rectangle adminCheck = { 700, 100, 25, 25 };
    DrawRectangleRec(adminCheck, LIGHTGRAY);
    if (isAdminFlag) DrawRectangle(adminCheck.x + 5, adminCheck.y + 5, 15, 15, DARKBLUE);
    DrawTextBold("Is Admin?", adminCheck.x + 35, adminCheck.y + 5, 20, BLACK);
    if (CheckCollisionPointRec(GetMousePosition(), (Rectangle){adminCheck.x, adminCheck.y, 150, 25}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) isAdminFlag = !isAdminFlag;

    const char* buttonText = (editIndex == -1) ? "Add User" : "Save Changes";
    DrawRectangleRec(addButton, (editIndex == -1) ? DARKGREEN : BLUE);
    DrawTextBold(buttonText, addButton.x + addButton.width/2 - MeasureText(buttonText, 20)/2, addButton.y + 5, 20, WHITE);

    // --- Draw User List ---
    Rectangle view = { 20, 150, GetScreenWidth() - 40, GetScreenHeight() - 210 };
    DrawRectangleRec(view, (Color){230, 230, 230, 255});
    DrawRectangleLinesEx(view, 1, DARKGRAY);

    // Handle scrolling
    const float itemHeight = 30.0f;
    const float contentHeight = num_accounts * itemHeight;
    if (CheckCollisionPointRec(GetMousePosition(), view)) {
        scroll.y += GetMouseWheelMove() * itemHeight;
        if (scroll.y > 0) scroll.y = 0;
        if (contentHeight > view.height && scroll.y < view.height - contentHeight) scroll.y = view.height - contentHeight;
        else if (contentHeight <= view.height) scroll.y = 0;
    }

    BeginScissorMode(view.x, view.y, view.width, view.height);
    
    // List Header
    DrawTextBold("Name", view.x + 20, view.y + 10, 20, DARKGRAY);
    DrawTextBold("Email", view.x + 300, view.y + 10, 20, DARKGRAY);
    DrawTextBold("Role", view.x + 650, view.y + 10, 20, DARKGRAY);
    DrawTextBold("Actions", view.x + 800, view.y + 10, 20, DARKGRAY);

    for (int i = 0; i < num_accounts; i++) {
        float y_pos = view.y + 40 + (i * itemHeight) + scroll.y;
        if (y_pos < view.y - itemHeight || y_pos > view.y + view.height) continue; // Culling

        Color textColor = (editIndex == i) ? BLUE : BLACK;
        DrawText(accounts[i].name, view.x + 20, y_pos, 20, textColor);
        DrawText(accounts[i].email, view.x + 300, y_pos, 20, textColor);

        const char* role = "User";
        if (is_superadmin(accounts[i].email)) role = "Superadmin";
        else if (accounts[i].isAdmin) role = "Admin";
        DrawText(role, view.x + 650, y_pos, 20, (strcmp(role,"User")==0) ? GRAY : DARKBLUE);

        // Action buttons
        if (!is_superadmin(accounts[i].email)) {
            Rectangle editBtn = { view.x + 800, y_pos, 60, 25 };
            Rectangle deleteBtn = { view.x + 870, y_pos, 70, 25 };

            DrawRectangleRec(editBtn, ORANGE); DrawText("Edit", editBtn.x + 15, editBtn.y + 5, 15, BLACK);
            DrawRectangleRec(deleteBtn, RED); DrawText("Delete", deleteBtn.x + 10, deleteBtn.y + 5, 15, WHITE);

            if (CheckCollisionPointRec(GetMousePosition(), editBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                editIndex = i;
                strcpy(nameBox.text, accounts[i].name); nameBox.charCount = strlen(nameBox.text);
                strcpy(emailBox.text, accounts[i].email); emailBox.charCount = strlen(emailBox.text);
                passwordBox.charCount = 0; passwordBox.text[0] = '\0'; // Clear password for editing
                isAdminFlag = accounts[i].isAdmin;
            }

            if (CheckCollisionPointRec(GetMousePosition(), deleteBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                for (int j = i; j < num_accounts - 1; j++) {
                    accounts[j] = accounts[j + 1];
                }
                num_accounts--;
                if (num_accounts > 0) accounts = realloc(accounts, num_accounts * sizeof(Account)); else { free(accounts); accounts = NULL; }
                save_all_accounts(accounts, num_accounts);
                needs_refresh = true;
                break;
            }
        }
    }
    EndScissorMode();

    // Back button
    DrawRectangleRec(backButton, LIGHTGRAY);
    DrawTextBold("Back", backButton.x + backButton.width/2 - MeasureText("Back", 20)/2, backButton.y + 10, 20, BLACK);

    EndDrawing();
}

// A helper function to manage text box input logic
static void HandleTextBox(TextBox *box) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Check if the mouse click is inside this text box
        box->active = CheckCollisionPointRec(GetMousePosition(), box->bounds);
    }

    if (box->active) {
        int key = GetCharPressed();
        // Allow characters from space to ~
        if ((key >= 32) && (key <= 126) && (box->charCount < 127)) {
            box->text[box->charCount] = (char)key;
            box->text[box->charCount + 1] = '\0';
            box->charCount++;
        }

        if (IsKeyPressed(KEY_BACKSPACE)) {
            box->charCount--;
            if (box->charCount < 0) box->charCount = 0;
            box->text[box->charCount] = '\0';
        }
        
        // Add a blinking cursor effect
        if ((((int)(GetTime()*2.0f)) % 2) == 0) {
            DrawText("|", box->bounds.x + 8 + MeasureText(box->text, 20), box->bounds.y + 12, 20, BLACK);
        }
    }
}
