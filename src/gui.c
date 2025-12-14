#include "raylib.h"
#include "match.h"
#include "teams.h"
#include "accounts.h" // Include accounts for user types
#include "tournament.h" // For MAX_MATCHES_IN_TOURNAMENT
#include <stdio.h> // For sprintf
#include <ctype.h> // For toupper()
#include <string.h>
#include <time.h> // For date and time
#include <math.h> // For cosf and sinf
#include "raymath.h"
#include "field_setups.h"

#define SILVER (Color){ 192, 192, 192, 255 }

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
    SCREEN_PLACEHOLDER,
    SCREEN_MANAGE_USERS,
    SCREEN_MATCH_SETUP,
    SCREEN_WC_SETUP
} GameScreen;

// Structure to hold all GUI state, including user info
typedef struct {
    GameScreen currentScreen;
    GameScreen previousScreen; // To enable a true "back" button
    UserType userType;
    char userEmail[128];
    char userName[64];
} GuiState;

// Structure to hold all game sounds
typedef struct {
    Sound bowling;
    Sound dot_ball;
    Sound edge;
    Sound firecrackers;
    Sound four;
    Sound single;
    Sound six;
    Sound toss;
    Sound bowled_wicket;
} GameSounds;

// Forward declarations for screen functions
static void ChangeScreen(GuiState *state, GameScreen newScreen);
static void UpdateDrawLoginScreen(GuiState *state);
static void UpdateDrawRegisterScreen(GuiState *state);
static void UpdateDrawAdminMenuScreen(GuiState *state);
static void UpdateDrawMainMenuScreen(GuiState *state, GameState *gameState);
static void InitializeAudience(void);
static void UpdateDrawGameplayScreen(GuiState *state, GameState *gameState, GameSounds *sounds);
static void UpdateDrawUmpiresScreen(GuiState *state);
static void UpdateDrawTeamsScreen(GuiState *state);
static void UpdateDrawPlaceholderScreen(GuiState *state, const char *title);
static void UpdateDrawManageUsersScreen(GuiState *state);
static void UpdateDrawMatchSetupScreen(GuiState *state);
static void UpdateDrawWcSetupScreen(GuiState *state, GameState *gameState, GameSounds *sounds);
static void DrawTextBold(const char *text, int posX, int posY, int fontSize, Color color);

// Helper for text boxes
static void HandleTextBox(TextBox *box);

// Helpers for gameplay screen
static void DrawPlayerFigure(Vector2 position, Color color, bool is_striker, const char* team_name, bool is_swinging);
static void DrawFielders(const Vector2 *fielder_positions, GameState *gameState, Vector2 fieldCenter, float fieldRadius, int dragging_fielder_idx);



int main(void)
{
    // Set a default window size and allow resizing
    const int screenWidth = 1280;
    const int screenHeight = 720;
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(screenWidth, screenHeight, "CricSim");
    InitAudioDevice();
    InitializeAudience(); // Initial setup of audience positions

    // Load all game sounds
    GameSounds sounds = {
        .bowling = LoadSound("audio/bowling.mp3"),
        .dot_ball = LoadSound("audio/dotball.mp3"),
        .edge = LoadSound("audio/edge.mp3"),
        .firecrackers = LoadSound("audio/firecrackers.mp3"),
        .four = LoadSound("audio/four_and_singles.mp3"),
        .single = LoadSound("audio/single_yes_call.mp3"),
        .six = LoadSound("audio/six.mp3"),
        .toss = LoadSound("audio/toss.mp3"),
        .bowled_wicket = LoadSound("audio/bowled_wicket.mp3")
    };
    
    // Match Game State Initialization
    GameState gameState = { 0 };
    Team teamA, teamB;
    
    initialize_dummy_teams(&teamA, &teamB);

    gameState.batting_team = &teamA;
    gameState.bowling_team = &teamB;
    gameState.max_overs = 50;
    gameState.fielding_setup = PP_AGGRESSIVE; // Initialize with default fielding
    gameState.gameplay_mode = GAMEPLAY_MODE_PLAYING; // Initialize gameplay mode
    gameState.inning_num = 1;

    // GUI State Initialization
    GuiState guiState = { 0 };
    guiState.currentScreen = SCREEN_LOGIN;
    guiState.previousScreen = SCREEN_LOGIN; // Initialize previous screen
    guiState.userType = USER_TYPE_NONE;

    create_saves_directory(); // Ensure the saves directory exists

    SetTargetFPS(60);

    // Main game loop
        while (!WindowShouldClose()) {
            if (IsWindowResized()) {
                InitializeAudience();
            }
    
        switch(guiState.currentScreen) {
            case SCREEN_LOGIN:
                UpdateDrawLoginScreen(&guiState);
                break;
            case SCREEN_REGISTER:
                UpdateDrawRegisterScreen(&guiState);
                break;
            case SCREEN_MAIN_MENU:
                UpdateDrawMainMenuScreen(&guiState, &gameState);
                break;
            case SCREEN_ADMIN_MENU:
                UpdateDrawAdminMenuScreen(&guiState);
                break;
            case SCREEN_GAMEPLAY:
                // We pass the gameState to the gameplay screen to be updated
                UpdateDrawGameplayScreen(&guiState, &gameState, &sounds);
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
            case SCREEN_PLACEHOLDER:
                UpdateDrawPlaceholderScreen(&guiState, "Coming Soon...");
                break;
            case SCREEN_MANAGE_USERS:
                UpdateDrawManageUsersScreen(&guiState);
                break;
            case SCREEN_MATCH_SETUP:
                UpdateDrawMatchSetupScreen(&guiState);
                break;
            case SCREEN_WC_SETUP:
                UpdateDrawWcSetupScreen(&guiState, &gameState, &sounds);
                break;
            default:
                break;
        }
    }

    if (guiState.currentScreen == SCREEN_GAMEPLAY) {
        save_game_state(&gameState, "Data/saves/resume.dat");
    }

    // Unload all sounds
    UnloadSound(sounds.bowling);
    UnloadSound(sounds.dot_ball);
    UnloadSound(sounds.edge);
    UnloadSound(sounds.firecrackers);
    UnloadSound(sounds.four);
    UnloadSound(sounds.single);
    UnloadSound(sounds.six);
    UnloadSound(sounds.toss);
    UnloadSound(sounds.bowled_wicket);

    CloseAudioDevice();
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

static void UpdateDrawMainMenuScreen(GuiState *state, GameState *gameState) {
    const int screenWidth = GetScreenWidth();
    const Rectangle resumeGameButton = { screenWidth/2 - 200, GetScreenHeight()/2 - 250, 400, 60 };
    const Rectangle playButton = { screenWidth/2 - 200, GetScreenHeight()/2 - 180, 400, 60 };
    const Rectangle teamsButton = { screenWidth/2 - 200, GetScreenHeight()/2 - 110, 400, 60 };
    const Rectangle umpiresButton = { screenWidth/2 - 200, GetScreenHeight()/2 - 40, 400, 60 };
    const Rectangle historyButton = { screenWidth/2 - 200, GetScreenHeight()/2 + 30, 400, 60 };
    const Rectangle logoutButton = { screenWidth/2 - 200, GetScreenHeight()/2 + 100, 400, 60 };

    Vector2 mousePoint = GetMousePosition();
    bool saveFileExists = FileExists("Data/saves/resume.dat");

    if (saveFileExists && CheckCollisionPointRec(mousePoint, resumeGameButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (load_game_state(gameState, "Data/saves/resume.dat")) {
            ChangeScreen(state, SCREEN_GAMEPLAY);
        }
    }
    if (CheckCollisionPointRec(mousePoint, playButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        ChangeScreen(state, SCREEN_MATCH_SETUP);
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
    DrawText(title, screenWidth/2 - MeasureText(title, 50)/2, GetScreenHeight()/2 - 320, 50, DARKGRAY);

    if (saveFileExists) {
        DrawRectangleRec(resumeGameButton, LIGHTGRAY);
        DrawText("Resume Game", resumeGameButton.x + resumeGameButton.width/2 - MeasureText("Resume Game", 20)/2, resumeGameButton.y + 15, 20, BLACK);
    }

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

static void DrawPlayerFigure(Vector2 position, Color color, bool is_striker, const char* team_name, bool is_swinging) {
    const float scale = 1.2f; // Adjust this to change player size
    const float head_radius = 6.0f * scale;
    const float body_height = 15.0f * scale;
    const float body_width = 12.0f * scale;
    const float limb_length = 10.0f * scale;
    const float limb_thick = 2.0f * scale;

    // Draw team name behind the player
    if (team_name) {
        DrawText(team_name, position.x - MeasureText(team_name, 10)/2, position.y - head_radius - 15, 10, DARKGRAY);
    }

    // Body
    Rectangle body = { position.x - body_width/2, position.y, body_width, body_height };
    DrawRectangleRec(body, color);

    // Head
    DrawCircle(position.x, position.y - head_radius, head_radius, color);

    if (is_striker) { // Batsman
        // Draw bat based on stance
        if (is_swinging) {
            DrawLineEx((Vector2){position.x, position.y + 5}, (Vector2){position.x - limb_length, position.y + 10}, limb_thick, color); // Swinging Bat
        } else {
            DrawLineEx((Vector2){position.x, position.y + 5}, (Vector2){position.x + limb_length, position.y + 5}, limb_thick, color); // Batting Stance
        }
        DrawLineEx((Vector2){position.x, position.y + body_height}, (Vector2){position.x - limb_length/2, position.y + body_height + limb_length}, limb_thick, color); // Left leg
        DrawLineEx((Vector2){position.x + limb_length/4, position.y + body_height}, (Vector2){position.x + limb_length, position.y + body_height + limb_length}, limb_thick, color); // Right leg
    } else { // Fielding/Bowling Stance
        DrawLineEx((Vector2){position.x - body_width/2, position.y}, (Vector2){position.x - body_width, position.y + limb_length}, limb_thick, color); // Left arm
        DrawLineEx((Vector2){position.x + body_width/2, position.y}, (Vector2){position.x + body_width, position.y + limb_length}, limb_thick, color); // Right arm
        DrawLineEx((Vector2){position.x, position.y + body_height}, (Vector2){position.x - limb_length/2, position.y + body_height + limb_length}, limb_thick, color); // Left leg
        DrawLineEx((Vector2){position.x, position.y + body_height}, (Vector2){position.x + limb_length/2, position.y + body_height + limb_length}, limb_thick, color); // Right leg
    }
}

static void DrawFielders(const Vector2 *fielder_positions, GameState *gameState, Vector2 fieldCenter, float fieldRadius, int dragging_fielder_idx) {
    float normalizedThirtyYardRadius = 0.45f; // Normalized radius for checks

    // Find the wicket-keeper to exclude them from general fielding positions
    int keeper_player_list_idx = -1; // Index within gameState->bowling_team->players
    for (int i = 0; i < gameState->bowling_team->num_players; i++) {
        if (gameState->bowling_team->players[i].is_wicketkeeper) {
            keeper_player_list_idx = i;
            break;
        }
    }
    // If no dedicated keeper, the last player is assumed to be the keeper (fallback)
    if (keeper_player_list_idx == -1 && gameState->bowling_team->num_players > 0) {
        keeper_player_list_idx = gameState->bowling_team->num_players - 1;
    }

    // Draw the wicket-keeper behind the stumps (fixed position)
    // This assumes the keeper is NOT one of the NUM_FIELDERS in fielder_positions



    // Determine powerplay rules for fielder coloring
    int max_outside = 0;
    if (gameState->current_powerplay == POWERPLAY_1) max_outside = 2;
    else if (gameState->current_powerplay == POWERPLAY_2) max_outside = 4;
    else if (gameState->current_powerplay == POWERPLAY_3) max_outside = 5;

    int outside_count_current_view = 0;
    for (int i = 0; i < NUM_FIELDERS; i++) {
        // Only count fielders (excluding bowler and keeper if they were part of the 9)
        // For now, assuming fielder_positions contains only the 9 fielders.
        Vector2 pos = fielder_positions[i];
        if (Vector2Length(pos) > normalizedThirtyYardRadius) {
            outside_count_current_view++;
        }
    }

    // Draw the other fielders from the provided positions
    for (int i = 0; i < NUM_FIELDERS; i++) {
        Vector2 pos_normalized = fielder_positions[i];
        Vector2 fielderScreenPos = {
            fieldCenter.x + pos_normalized.x * (fieldRadius - 15), // Place 15 units inside the green field
            fieldCenter.y + pos_normalized.y * (fieldRadius - 15)
        };
        
        Color fielderColor = RED; // Default color
        if (gameState->gameplay_mode == GAMEPLAY_MODE_CUSTOM_FIELDING && i == dragging_fielder_idx) {
            fielderColor = BLUE; // Highlight dragged fielder
        } else if (Vector2Length(pos_normalized) > normalizedThirtyYardRadius) { // If fielder is outside 30-yard circle
            if (outside_count_current_view > max_outside) {
                fielderColor = YELLOW; // Highlight if too many outside
            }
        }

        DrawPlayerFigure(fielderScreenPos, fielderColor, false, NULL, false);
    }
    // Draw the wicket-keeper behind the stumps (fixed position, assuming striker at right end of pitch)
    Vector2 keeperScreenPos = { fieldCenter.x + 190, fieldCenter.y };
    if (keeper_player_list_idx != -1 && keeper_player_list_idx < gameState->bowling_team->num_players) {
        DrawPlayerFigure(keeperScreenPos, ORANGE, false, gameState->bowling_team->players[keeper_player_list_idx].name, false);
    }
}

// Structure to hold a single audience member's data
typedef struct {
    Vector2 position;
    Color baseColor;
} AudienceMember;

static AudienceMember audience[1500];






// Function to initialize or re-initialize audience positions based on current screen size
static void InitializeAudience() {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const Vector2 fieldCenter = { screenWidth / 2.0f, screenHeight / 2.0f + 50 };
    const float boundaryRadius = screenWidth < screenHeight ? screenWidth * 0.35f : screenHeight * 0.35f;
    const float standsRadius = boundaryRadius + 70; // Audience stands

    for (int i = 0; i < 1500; i++) {
        float angle = (float)rand() / (float)RAND_MAX * 360.0f;
        float dist_variation = (float)rand() / (float)RAND_MAX * 60.0f;
        float dist = standsRadius + dist_variation;
        audience[i].position = (Vector2){ fieldCenter.x + cosf(angle) * dist, fieldCenter.y + sinf(angle) * dist };
        
        // Divide audience into two halves for each team
        if (angle > 180) {
            audience[i].baseColor = BLUE; // Team A supporters
        } else {
            audience[i].baseColor = RED; // Team B supporters
        }
    }
}

static void UpdateDrawGameplayScreen(GuiState *state, GameState *gameState, GameSounds *sounds) {
    const Rectangle backButton = { GetScreenWidth() - 170, GetScreenHeight() - 60, 150, 40 };

    if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        // Go back to the previous screen (likely the main menu)
        save_game_state(gameState, "Data/saves/resume.dat");
        ChangeScreen(state, state->previousScreen);
    }

    // --- Fielding Setup UI ---
    const char* field_setup_names[] = {"PP_Aggressive", "PP_Defensive", "MO_Normal", "MO_Defensive", "DO_Defensive"};
    Rectangle fieldSetupButton = { GetScreenWidth() - 200, 20, 180, 40 };
    Rectangle editFieldButton = { GetScreenWidth() - 200, 70, 180, 40 };

    if (gameState->gameplay_mode == GAMEPLAY_MODE_PLAYING) {
        DrawRectangleRec(fieldSetupButton, DARKBLUE);
        char fieldButtonText[64];
        sprintf(fieldButtonText, "Field: %s", field_setup_names[gameState->fielding_setup]);
        DrawText(fieldButtonText, fieldSetupButton.x + 10, fieldSetupButton.y + 10, 20, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), fieldSetupButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (gameState->current_powerplay == POWERPLAY_1) {
                if (gameState->fielding_setup == PP_AGGRESSIVE) {
                    gameState->fielding_setup = PP_DEFENSIVE;
                } else {
                    gameState->fielding_setup = PP_AGGRESSIVE;
                }
            } else {
                gameState->fielding_setup = (FieldingSetup)((gameState->fielding_setup + 1) % FIELD_SETUP_COUNT);
            }
        }

        DrawRectangleRec(editFieldButton, GREEN);
        DrawText("Edit Field", editFieldButton.x + 10, editFieldButton.y + 10, 20, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), editFieldButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            gameState->gameplay_mode = GAMEPLAY_MODE_CUSTOM_FIELDING;
            // Copy current setup to custom setup
            const Vector2 *current_base_setup = setups[gameState->fielding_setup];
            for (int i = 0; i < NUM_FIELDERS; i++) {
                gameState->custom_field_setup[i] = current_base_setup[i];
            }
        }
    } else { // GAMEPLAY_MODE_CUSTOM_FIELDING
        DrawRectangleRec(fieldSetupButton, GRAY); // Disabled when editing
        DrawText("Field: Custom", fieldSetupButton.x + 10, fieldSetupButton.y + 10, 20, LIGHTGRAY);

        DrawRectangleRec(editFieldButton, RED);
        DrawText("Apply Field", editFieldButton.x + 10, editFieldButton.y + 10, 20, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), editFieldButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            gameState->gameplay_mode = GAMEPLAY_MODE_PLAYING;
            // The custom setup is now the active one for the remainder of gameplay
            // No need to copy back to fielding_setup, DrawFielders will use custom_field_setup
        }
    }

    // --- Pre-calculate positions ---
    Vector2 fieldCenter = { GetScreenWidth() / 2.0f, 120 + (GetScreenHeight() - 220) / 2.0f };
    float fieldRadius = (GetScreenHeight() - 240) / 2.0f;
    if (fieldRadius > (GetScreenWidth() - 100) / 2.0f) fieldRadius = (GetScreenWidth() - 100) / 2.0f;

    // --- Gameplay Animation & State Variables ---
    static GameplayPhase currentPhase = PHASE_IDLE;
    static double celebration_end_time = 0.0;
    static Vector2 bowlerAnimPos;
    static Vector2 ballPos;
    static Vector2 ballTargetPos;
    static float animTimer = 0.0f;
    static bool showRunButton = false;
    static float boundaryAnimTimer = 0.0f; // Timer for boundary animation
    static float fielderPauseTimer = 0.0f; // New timer for fielder's pause
            static int runsThisBall = 0;
            static Vector2 strikerAnimPos, nonStrikerAnimPos;    static float runAnimTimer = 0.0f;
    static Vector2 fielderRunPos;
    static int nearestFielderIndex = -1;
    static int dragging_fielder_idx = -1; // Index of the fielder being dragged (-1 if none)
    static Vector2 drag_offset; // Offset from fielder's center to mouse position when dragging started
    static double playMissMessageEndTime = 0.0; // For "Play and Miss!" message
    static char outcomeMessage[32] = {0};
    static double outcomeMessageEndTime = 0.0;
    static float ballAltitude = 0.0f;
    static float ballVelocityY = 0.0f;
    static bool match_over_sound_played = false;

    bool isGameOver = (gameState->overs_completed >= gameState->max_overs || gameState->wickets >= 10);

    if (currentPhase == PHASE_IDLE && !isGameOver && IsKeyPressed(KEY_SPACE)) {
        currentPhase = PHASE_BOWLER_RUNUP;
        PlaySound(sounds->bowling);
        bowlerAnimPos = (Vector2){ GetScreenWidth() / 2.0f - 250, GetScreenHeight() / 2.0f }; // Start of run-up
        animTimer = 0.0f;
        showRunButton = false;
        strikerAnimPos = (Vector2){ fieldCenter.x + 180, fieldCenter.y };
        nonStrikerAnimPos = (Vector2){ fieldCenter.x - 180, fieldCenter.y };
        runsThisBall = 0;
        fielderPauseTimer = 0.0f; // Reset pause timer for a new ball
        boundaryAnimTimer = 0.0f; // Reset boundary animation timer
    }


    BeginDrawing();
    ClearBackground(DARKGRAY); // A darker background for a stadium feel

    
    // --- Draw Stadium Environment ---
    
    float boundaryRadius = fieldRadius + 15; // Boundary rope
    float standsRadius = boundaryRadius + 70; // Audience stands

    // Draw Stands
    DrawCircleV(fieldCenter, standsRadius, (Color){60, 60, 60, 255});



    // Draw the audience
    bool is_celebrating = (GetTime() < celebration_end_time);
    for (int i = 0; i < 1500; i++) {
        Color finalColor = audience[i].baseColor;
        // If celebrating, make some of the crowd flash
        if (is_celebrating && (rand() % 5 == 0)) {
            finalColor = (rand() % 2 == 0) ? YELLOW : WHITE;
        }
        DrawCircleV(audience[i].position, 2, finalColor);
    }

    // Draw Floodlights
    DrawRectangle(50, 50, 20, 100, GRAY); DrawCircle(60, 45, 15, YELLOW);
    DrawRectangle(GetScreenWidth() - 70, 50, 20, 100, GRAY); DrawCircle(GetScreenWidth() - 60, 45, 15, YELLOW);
    DrawRectangle(50, GetScreenHeight() - 150, 20, 100, GRAY); DrawCircle(60, GetScreenHeight() - 155, 15, YELLOW);
    DrawRectangle(GetScreenWidth() - 70, GetScreenHeight() - 150, 20, 100, GRAY); DrawCircle(GetScreenWidth() - 60, GetScreenHeight() - 155, 15, YELLOW);

    // Draw Entry Gate
    DrawRectangle(GetScreenWidth()/2 - 50, (int)(fieldCenter.y + standsRadius), 100, 40, (Color){40, 40, 40, 255});

    // --- Draw the Cricket Field ---
    DrawCircleV(fieldCenter, fieldRadius, DARKGREEN);
    DrawCircleLinesV(fieldCenter, boundaryRadius, WHITE); // Boundary Rope

    // Draw 30-yard circle
    float thirtyYardRadius = fieldRadius * 0.45f;
    DrawCircleLinesV(fieldCenter, thirtyYardRadius, Fade(WHITE, 0.3f));

    // Draw the pitch
    Rectangle pitch = { fieldCenter.x - 200, fieldCenter.y - 25, 400, 50 };
    DrawRectangleRec(pitch, DARKBROWN);
    DrawRectangle(pitch.x + 20, pitch.y, 5, pitch.height, LIGHTGRAY); // Crease
    DrawRectangle(pitch.x + pitch.width - 25, pitch.y, 5, pitch.height, LIGHTGRAY); // Crease

    // Display Powerplay info
    const char* pp_text = "";
    int max_outside = 0;
    switch(gameState->current_powerplay) {
        case POWERPLAY_1: pp_text = "PP1 (1-10)"; max_outside = 2; break;
        case POWERPLAY_2: pp_text = "PP2 (11-40)"; max_outside = 4; break;
        case POWERPLAY_3: pp_text = "PP3 (41-50)"; max_outside = 5; break;
    }
    DrawText(TextFormat("Powerplay: %s (%d outside max)", pp_text, max_outside), 20, 100, 20, GRAY);


    // --- Player Positions ---
    Vector2 strikerEnd = { fieldCenter.x + 180, fieldCenter.y }; // Use a consistent name
    Vector2 nonStrikerEnd = { fieldCenter.x - 180, fieldCenter.y }; // Use a consistent name
    Vector2 bowlerDefaultPos = { fieldCenter.x - 250, fieldCenter.y };

    
    // --- Animation State Machine ---
    switch (currentPhase) {
        case PHASE_BOWLER_RUNUP:
            animTimer += GetFrameTime();
            bowlerAnimPos.x = Lerp(bowlerDefaultPos.x, fieldCenter.x - 220, animTimer / 1.0f); // 1-second run-up
            if (animTimer >= 1.0f) {
                currentPhase = PHASE_BALL_TRAVEL;
                ballPos = (Vector2){bowlerAnimPos.x, bowlerAnimPos.y - 10};
                animTimer = 0.0f;
            }
            break;
        case PHASE_BALL_TRAVEL:
            animTimer += GetFrameTime();
            ballPos = Vector2Lerp( (Vector2){fieldCenter.x - 220, fieldCenter.y - 10}, (Vector2){strikerEnd.x, strikerEnd.y - 10}, animTimer / 0.5f); // 0.5-second travel
            if (animTimer >= 0.5f) {
                currentPhase = PHASE_BATSMAN_SWING;
                animTimer = 0.0f;
            }
            break;
        case PHASE_BATSMAN_SWING:
    animTimer += GetFrameTime();
    if (animTimer >= 0.2f) { // 0.2-second swing
        // --- NEW SKILL AND FIELDING BASED SIMULATION ---
        Player *striker = &gameState->batting_team->players[gameState->striker_idx];
        Player *bowler = &gameState->bowling_team->players[gameState->bowler_idx];

        // 1. Skill difference & Timing
        int skill_diff = striker->batting_skill - bowler->bowling_skill;
        int timing = (rand() % 41) - 20; // -20 (very poor) to +20 (perfect)
        int final_shot_quality = skill_diff + timing;

        // 2. Introduce "Play and Miss" dot ball chance
        // A "play and miss" means no runs and no wicket, just a dot ball.
        int play_and_miss_chance = 60 - (striker->batting_skill - bowler->bowling_skill) / 3;
        if (play_and_miss_chance < 5) play_and_miss_chance = 5; // Minimum 5% chance
        if (play_and_miss_chance > 60) play_and_miss_chance = 60; // Maximum 60% chance

        if (rand() % 100 < play_and_miss_chance) {
            // It's a play and miss dot ball
            runsThisBall = 0;
            // Inlined ball update logic
            gameState->balls_bowled_in_over++;
            if (gameState->balls_bowled_in_over >= 6) {
                gameState->overs_completed++;
                gameState->balls_bowled_in_over = 0;
                // Rotate strike at end of over
                int temp = gameState->striker_idx;
                gameState->striker_idx = gameState->non_striker_idx;
                gameState->non_striker_idx = temp;

                // Select a new bowler - get current bowler's index
                Player *current_bowler_player = &gameState->bowling_team->players[gameState->bowler_idx]; // Get current bowler
                int current_bowler_p_idx = -1;
                for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                    if(&gameState->bowling_team->players[i] == current_bowler_player) {
                        current_bowler_p_idx = i;
                        break;
                    }
                }
                // Find the next bowler
                Player *new_bowler = get_bowler(gameState->bowling_team, current_bowler_p_idx);
                // Update bowler_idx in GameState
                for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                     if(&gameState->bowling_team->players[i] == new_bowler) {
                        gameState->bowler_idx = i;
                        break;
                    }
                }
            }
            // Log ball data for play and miss dot
            log_ball_data(gameState, 1, gameState->overs_completed, gameState->balls_bowled_in_over,
                          striker, bowler, 0, OUTCOME_DOT, NULL, NULL);

            playMissMessageEndTime = GetTime() + 1.5; // Show message for 1.5 seconds
            PlaySound(sounds->dot_ball);
            currentPhase = PHASE_IDLE; // Reset for the next ball
            animTimer = 0.0f;
            return; // Exit, as this ball is completed.
        }
        // 2. Wicket chance on poor timing/skill
        if (final_shot_quality < -25) {
            gameState->wickets++;
            bowler->total_wickets++;
            strcpy(outcomeMessage, "WICKET!");
            outcomeMessageEndTime = GetTime() + 2.0;
            // Move to next batsman
            gameState->striker_idx = (gameState->striker_idx > gameState->non_striker_idx) ? gameState->striker_idx + 1 : gameState->non_striker_idx + 1;

            // Determine dismissal method (50/50 for now)
            const char *dismissal_type;
            Player *dismissal_fielder = NULL;
            if (rand() % 2 == 0) {
                dismissal_type = "Bowled";
                PlaySound(sounds->bowled_wicket);
            } else {
                dismissal_type = "Caught (Simulated)";
                PlaySound(sounds->edge);
            }

            // Inlined ball update logic
            gameState->balls_bowled_in_over++;
            if (gameState->balls_bowled_in_over >= 6) {
                gameState->overs_completed++;
                gameState->balls_bowled_in_over = 0;
                // Rotate strike at end of over
                int temp = gameState->striker_idx;
                gameState->striker_idx = gameState->non_striker_idx;
                gameState->non_striker_idx = temp;

                // Select a new bowler - get current bowler's index
                int current_bowler_p_idx = -1;
                for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                    if(&gameState->bowling_team->players[i] == bowler) {
                        current_bowler_p_idx = i;
                        break;
                    }
                }
                // Find the next bowler
                Player *new_bowler = get_bowler(gameState->bowling_team, current_bowler_p_idx);
                // Update bowler_idx in GameState
                for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                     if(&gameState->bowling_team->players[i] == new_bowler) {
                        gameState->bowler_idx = i;
                        break;
                    }
                }
            }
            // Log ball data
            log_ball_data(gameState, 1, gameState->overs_completed, gameState->balls_bowled_in_over,
                          striker, bowler, 0, OUTCOME_WICKET, dismissal_type, dismissal_fielder);

            currentPhase = PHASE_IDLE; // Back to idle if out
        } else {
            // 3. Determine shot placement
            float angle = (float)(GetRandomValue(-135, 135)) * DEG2RAD;
            // Distance is based on shot quality
            // Adjusted dist_multiplier for realism and reduced 6s frequency
            float dist_multiplier = 0.3f + (float)(final_shot_quality + 30) / 100.0f; // Range roughly 0.36 to 0.8
            float dist = (fieldRadius * 0.8f) * dist_multiplier;
            dist += (rand() % (int)(fieldRadius * 0.05f)); // Reduced random boost

            ballTargetPos = (Vector2){ fieldCenter.x + cosf(angle) * dist, fieldCenter.y + sinf(angle) * dist };
            ballPos = strikerEnd;
            runsThisBall = 0;
            currentPhase = PHASE_BALL_IN_FIELD; // Assume ball is in field initially

            // 4. Check for Boundary
            if (Vector2Distance(fieldCenter, ballTargetPos) > boundaryRadius) {
                if (dist > boundaryRadius + 60) { // Increased difficulty for a direct hit for 6
                    runsThisBall = 6;
                    strcpy(outcomeMessage, "SIX!");
                    outcomeMessageEndTime = GetTime() + 2.0;
                    ballAltitude = 0.0f;
                    ballVelocityY = 150.0f;
                    PlaySound(sounds->six);
                } else { // Bounces to boundary for 4
                    runsThisBall = 4;
                    strcpy(outcomeMessage, "FOUR!");
                    outcomeMessageEndTime = GetTime() + 2.0;
                    ballAltitude = 0.0f;
                    ballVelocityY = 80.0f;
                    PlaySound(sounds->four);
                }
                gameState->total_runs += runsThisBall;
                striker->total_runs += runsThisBall;
                celebration_end_time = GetTime() + 2.0;
                currentPhase = PHASE_BOUNDARY_ANIMATION;
                animTimer = 0.0f;
            } else {
                // 5. Find nearest fielder if not a boundary
                const Vector2 *setup = get_field_setup(gameState->fielding_setup, striker);
                float nearestDist = 10000.0f;
                nearestFielderIndex = -1;
                
                for (int i = 0; i < NUM_FIELDERS; i++) {
                    Vector2 pos_normalized = setup[i];
                    Vector2 fielderBasePos = { 
                        fieldCenter.x + pos_normalized.x * (fieldRadius - 15),
                        fieldCenter.y + pos_normalized.y * (fieldRadius - 15)
                    };
                    float d = Vector2Distance(fielderBasePos, ballTargetPos);
                    if (d < nearestDist) {
                        nearestDist = d;
                        fielderRunPos = fielderBasePos;
                        nearestFielderIndex = i;
                    }
                }

                // 6. Determine outcome based on fielder distance
                float fielding_skill = gameState->bowling_team->players[nearestFielderIndex].fielding_skill;
                float effective_distance = nearestDist - (fielding_skill / 10.0f); // Good fielders cover more ground

                if (effective_distance < 15.0f && final_shot_quality < 0) { // CATCH

                    gameState->wickets++;
                    bowler->total_wickets++;
                    strcpy(outcomeMessage, "WICKET!");
                    outcomeMessageEndTime = GetTime() + 2.0;
                    PlaySound(sounds->edge);
                    gameState->striker_idx = (gameState->striker_idx > gameState->non_striker_idx) ? gameState->striker_idx + 1 : gameState->non_striker_idx + 1;
                    
                    // Inlined ball update logic
                    gameState->balls_bowled_in_over++;
                    if (gameState->balls_bowled_in_over >= 6) {
                        gameState->overs_completed++;
                        gameState->balls_bowled_in_over = 0;
                        // Rotate strike at end of over
                        int temp = gameState->striker_idx;
                        gameState->striker_idx = gameState->non_striker_idx;
                        gameState->non_striker_idx = temp;

                        // Select a new bowler - get current bowler's index
                        int current_bowler_p_idx = -1;
                        for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                            if(&gameState->bowling_team->players[i] == bowler) {
                                current_bowler_p_idx = i;
                                break;
                            }
                        }
                        // Find the next bowler
                        Player *new_bowler = get_bowler(gameState->bowling_team, current_bowler_p_idx);
                        // Update bowler_idx in GameState
                        for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                             if(&gameState->bowling_team->players[i] == new_bowler) {
                                gameState->bowler_idx = i;
                                break;
                            }
                        }
                    }
                    // Log ball data for catch
                    // Check nearestFielderIndex validity
                    Player *fielder_out = NULL;
                    if (nearestFielderIndex >= 0 && nearestFielderIndex < gameState->bowling_team->num_players) {
                        fielder_out = &gameState->bowling_team->players[nearestFielderIndex];
                    }
                                        log_ball_data(gameState, 1, gameState->overs_completed, gameState->balls_bowled_in_over,
                                                      striker, bowler, 0, OUTCOME_WICKET, "Caught", fielder_out);                    showRunButton = true;
                    // The ball will be stopped quickly, limiting runs. This is handled by the fielder reaching the ball.
                } else { // In a gap
                    showRunButton = true;
                }
            }
        }
        animTimer = 0.0f;
    }
    break;
        case PHASE_BALL_IN_FIELD:
            // Animate ball and fielder moving to the target position
            {
                bool ballReachedTarget = (Vector2Distance(ballPos, ballTargetPos) < 5.0f);
                bool fielderReachedTarget = (Vector2Distance(fielderRunPos, ballTargetPos) < 5.0f);

                if (!ballReachedTarget) {
                    ballPos = Vector2Lerp(ballPos, ballTargetPos, GetFrameTime() * 2.0f);
                }
                if (!fielderReachedTarget) {
                    fielderRunPos = Vector2MoveTowards(fielderRunPos, ballTargetPos, 200.0f * GetFrameTime());
                }

                if (fielderReachedTarget) {
                    showRunButton = false; // Stop allowing runs

                    if (fielderPauseTimer == 0.0f) { // Start the pause timer only once
                        fielderPauseTimer = GetTime();
                    }

                    if (GetTime() - fielderPauseTimer >= 1.0f) { // After 1 second pause
                        if (runsThisBall > 0) {
                            sprintf(outcomeMessage, "%d RUN%s", runsThisBall, (runsThisBall > 1) ? "S" : "");
                            outcomeMessageEndTime = GetTime() + 2.0;
                        }
                        // Inlined ball update logic
                        gameState->balls_bowled_in_over++;
                        if (gameState->balls_bowled_in_over >= 6) {
                            gameState->overs_completed++;
                            gameState->balls_bowled_in_over = 0;
                            // Rotate strike at end of over
                            int temp = gameState->striker_idx;
                            gameState->striker_idx = gameState->non_striker_idx;
                            gameState->non_striker_idx = temp;

                            // Select a new bowler - get current bowler's index
                            Player *bowler = &gameState->bowling_team->players[gameState->bowler_idx]; // Get current bowler
                            int current_bowler_p_idx = -1;
                            for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                                if(&gameState->bowling_team->players[i] == bowler) {
                                    current_bowler_p_idx = i;
                                    break;
                                }
                            }
                            // Find the next bowler
                            Player *new_bowler = get_bowler(gameState->bowling_team, current_bowler_p_idx);
                            // Update bowler_idx in GameState
                            for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                                 if(&gameState->bowling_team->players[i] == new_bowler) {
                                    gameState->bowler_idx = i;
                                    break;
                                }
                            }
                        }
                        // Log ball data for fielded ball
                        log_ball_data(gameState, 1, gameState->overs_completed, gameState->balls_bowled_in_over,
                                      &gameState->batting_team->players[gameState->striker_idx], // Striker
                                      &gameState->bowling_team->players[gameState->bowler_idx], // Bowler
                                      runsThisBall, (runsThisBall > 0) ? OUTCOME_RUNS : OUTCOME_DOT, NULL, NULL);

                        currentPhase = PHASE_IDLE; // Reset for the next ball
                    }
                }
            }
            break;
        case PHASE_BOUNDARY_ANIMATION:
            // Animate ball to the boundary
            if (Vector2Distance(ballPos, ballTargetPos) > 5.0f) {
                ballPos = Vector2Lerp(ballPos, ballTargetPos, GetFrameTime() * 3.0f);
            }

            // Update ball altitude for 3D effect
            ballAltitude += ballVelocityY * GetFrameTime();
            ballVelocityY -= 300.0f * GetFrameTime(); // Gravity

            if (runsThisBall == 4) { // Four animation with bounces
                if (ballAltitude < 0) {
                    ballAltitude = 0;
                    ballVelocityY *= -0.6f; // Bounce with some energy loss
                    if (fabs(ballVelocityY) < 10.0f) ballVelocityY = 0; // Stop bouncing
                }
            } else { // Six animation
                if (ballAltitude < 0) {
                    ballAltitude = 0;
                    ballVelocityY = 0;
                }
            }

            boundaryAnimTimer += GetFrameTime();
            if (boundaryAnimTimer >= 2.5f) { // Increased timer for better animation
                // Inlined ball update logic
                gameState->balls_bowled_in_over++;
                if (gameState->balls_bowled_in_over >= 6) {
                    gameState->overs_completed++;
                    gameState->balls_bowled_in_over = 0;
                    // Rotate strike at end of over
                    int temp = gameState->striker_idx;
                    gameState->striker_idx = gameState->non_striker_idx;
                    gameState->non_striker_idx = temp;

                    // Select a new bowler - get current bowler's index
                    Player *bowler = &gameState->bowling_team->players[gameState->bowler_idx]; // Get current bowler
                    int current_bowler_p_idx = -1;
                    for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                        if(&gameState->bowling_team->players[i] == bowler) {
                            current_bowler_p_idx = i;
                            break;
                        }
                    }
                    // Find the next bowler
                    Player *new_bowler = get_bowler(gameState->bowling_team, current_bowler_p_idx);
                    // Update bowler_idx in GameState
                    for(int i=0; i<gameState->bowling_team->num_players; ++i) {
                         if(&gameState->bowling_team->players[i] == new_bowler) {
                            gameState->bowler_idx = i;
                            break;
                        }
                    }
                }
                // Log ball data for boundary
                log_ball_data(gameState, 1, gameState->overs_completed, gameState->balls_bowled_in_over,
                              &gameState->batting_team->players[gameState->striker_idx], // Striker
                              &gameState->bowling_team->players[gameState->bowler_idx], // Bowler
                              runsThisBall, OUTCOME_RUNS, NULL, NULL);

                currentPhase = PHASE_IDLE; // Allow next ball
                boundaryAnimTimer = 0.0f;
            }
            break;
        case PHASE_BATSMAN_RUNNING:
            runAnimTimer += GetFrameTime();
            // Determine which way they are running based on odd/even runs
            if (runsThisBall % 2 != 0) { // 1st, 3rd run etc.
                strikerAnimPos = Vector2Lerp(strikerEnd, nonStrikerEnd, runAnimTimer / 1.5f);
                nonStrikerAnimPos = Vector2Lerp(nonStrikerEnd, strikerEnd, runAnimTimer / 1.5f);
            } else { // 2nd, 4th run etc.
                strikerAnimPos = Vector2Lerp(nonStrikerEnd, strikerEnd, runAnimTimer / 1.5f);
                nonStrikerAnimPos = Vector2Lerp(strikerEnd, nonStrikerEnd, runAnimTimer / 1.5f);
            }

            if (runAnimTimer >= 1.2f) {
                currentPhase = PHASE_BALL_IN_FIELD; // Go back to fielding phase to allow another run
            }
            break;
        case PHASE_INNINGS_BREAK:
            if (IsKeyPressed(KEY_SPACE)) {
                // Set up for 2nd innings
                gameState->inning_num = 2;
                gameState->target = gameState->total_runs + 1;
                
                // Swap teams
                Team *temp_team = gameState->batting_team;
                gameState->batting_team = gameState->bowling_team;
                gameState->bowling_team = temp_team;

                char temp_tag[MAX_TEAM_TAG_LEN];
                strcpy(temp_tag, gameState->batting_team_tag);
                strcpy(gameState->batting_team_tag, gameState->bowling_team_tag);
                strcpy(gameState->bowling_team_tag, temp_tag);

                // Reset state
                gameState->total_runs = 0;
                gameState->wickets = 0;
                gameState->overs_completed = 0;
                gameState->balls_bowled_in_over = 0;
                gameState->striker_idx = 0;
                gameState->non_striker_idx = 1;
                gameState->bowler_idx = -1; // Will be selected before first ball

                currentPhase = PHASE_IDLE;
            }
            break;
        case PHASE_MATCH_OVER:
            if (!match_over_sound_played) {
                PlaySound(sounds->firecrackers);
                match_over_sound_played = true;
            }
            // Match is finished, do nothing until user exits
            break;
        default: // PHASE_IDLE or PHASE_INNINGS_OVER
            bowlerAnimPos = bowlerDefaultPos;
            strikerAnimPos = strikerEnd;
            nonStrikerAnimPos = nonStrikerEnd;
            match_over_sound_played = false; // Reset for next match
            break;
    }

    // --- Handle RUN button logic ---
    Rectangle runButton = { GetScreenWidth() - 180, GetScreenHeight() / 2 - 25, 160, 50 };
    if (showRunButton) {
        DrawRectangleRec(runButton, DARKGREEN);
        DrawText("RUN!", runButton.x + 50, runButton.y + 15, 20, WHITE);
        if (CheckCollisionPointRec(GetMousePosition(), runButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            runsThisBall++;
            gameState->total_runs++; // This should be here
            PlaySound(sounds->single);
            // Trigger the running animation
            currentPhase = PHASE_BATSMAN_RUNNING;
            runAnimTimer = 0.0f;

            // Celebrate audience
            if (runsThisBall > 0) {
                celebration_end_time = GetTime() + 1.0;
            }

            // TODO: Add run-out logic here
        }
    }
    
    // --- Draw Runs This Ball ---
    char runsText[32];
    sprintf(runsText, "This Ball: %d", runsThisBall);
    DrawText(runsText, GetScreenWidth() - 200, GetScreenHeight() - 40, 20, YELLOW);

    // --- Draw Players ---
    Player *striker = &gameState->batting_team->players[gameState->striker_idx];
    Player *non_striker = &gameState->batting_team->players[gameState->non_striker_idx];
    Player *bowler = &gameState->bowling_team->players[gameState->bowler_idx];

    // --- Draw Fielders (excluding the one who is running) ---
    const Vector2 *drawing_setup_positions = NULL;
    if (gameState->gameplay_mode == GAMEPLAY_MODE_CUSTOM_FIELDING) {
        drawing_setup_positions = gameState->custom_field_setup;
    } else {
        // In playing mode, get the standard setup
        drawing_setup_positions = get_field_setup(gameState->fielding_setup, striker);
    }
    
    // Pass the drawing_setup to the DrawFielders helper.
    // Also pass dragging_fielder_idx
    DrawFielders(drawing_setup_positions, gameState, fieldCenter, fieldRadius, dragging_fielder_idx);

    // --- Fielder Dragging Logic (only in custom fielding mode) ---
    if (gameState->gameplay_mode == GAMEPLAY_MODE_CUSTOM_FIELDING) {
        Vector2 mouse_pos = GetMousePosition();
        float player_size_for_click = 20.0f; // Approx size for click detection
        const float fielder_collision_min_dist = 20.0f; // Minimum distance between fielders in screen pixels

        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            for (int i = 0; i < NUM_FIELDERS; i++) {
                // Convert normalized position to screen position for click detection
                Vector2 pos_normalized = drawing_setup_positions[i];
                Vector2 fielderScreenPos = {
                    fieldCenter.x + pos_normalized.x * (fieldRadius - 15),
                    fieldCenter.y + pos_normalized.y * (fieldRadius - 15)
                };

                if (CheckCollisionPointCircle(mouse_pos, fielderScreenPos, player_size_for_click)) {
                    dragging_fielder_idx = i;
                    drag_offset = Vector2Subtract(mouse_pos, fielderScreenPos);
                    break;
                }
            }
        }

        if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            dragging_fielder_idx = -1;
        }

        if (dragging_fielder_idx != -1 && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            Vector2 new_screen_pos = Vector2Subtract(mouse_pos, drag_offset);
            
            // Convert screen position back to normalized field coordinates
            // This is inverse of: fielderScreenPos.x = fieldCenter.x + pos_normalized.x * (fieldRadius - 15)
            // pos_normalized.x = (new_screen_pos.x - fieldCenter.x) / (fieldRadius - 15)
            Vector2 new_normalized_pos;
            new_normalized_pos.x = (new_screen_pos.x - fieldCenter.x) / (fieldRadius - 15);
            new_normalized_pos.y = (new_screen_pos.y - fieldCenter.y) / (fieldRadius - 15);

            // Clamp normalized position to stay within field boundaries (approx -1.0 to 1.0)
            float current_dist_from_center = Vector2Length(new_normalized_pos);
            if (current_dist_from_center > 1.0f) {
                new_normalized_pos = Vector2Scale(new_normalized_pos, 1.0f / current_dist_from_center);
            }
            // Ensure fielder stays outside a minimum inner circle (e.g., pitch area)
            float min_fielder_distance = 0.2f; // Normalized minimum distance from field center
            if (current_dist_from_center < min_fielder_distance) {
                 new_normalized_pos = Vector2Scale(Vector2Normalize(new_normalized_pos), min_fielder_distance);
            }


            // --- Collision Detection ---
            bool collision = false;
            for (int i = 0; i < NUM_FIELDERS; i++) {
                if (i == dragging_fielder_idx) continue;

                // Calculate screen position for the other fielder
                Vector2 other_fielder_normalized_pos = drawing_setup_positions[i];
                Vector2 otherFielderScreenPos = {
                    fieldCenter.x + other_fielder_normalized_pos.x * (fieldRadius - 15),
                    fieldCenter.y + other_fielder_normalized_pos.y * (fieldRadius - 15)
                };

                if (Vector2Distance(new_screen_pos, otherFielderScreenPos) < fielder_collision_min_dist) {
                    collision = true;
                    break;
                }
            }

            if (!collision) {
                gameState->custom_field_setup[dragging_fielder_idx] = new_normalized_pos;
            }
        }
    }

    // Draw main players
    DrawPlayerFigure(strikerAnimPos, BLUE, true, gameState->batting_team->name, currentPhase == PHASE_BATSMAN_SWING);
    DrawText(striker->name, strikerAnimPos.x - MeasureText(striker->name, 10)/2, strikerAnimPos.y + 30, 10, WHITE);

    DrawPlayerFigure(nonStrikerAnimPos, BLUE, true, gameState->batting_team->name, false);
    DrawText(non_striker->name, nonStrikerAnimPos.x - MeasureText(non_striker->name, 10)/2, nonStrikerAnimPos.y + 30, 10, WHITE);

    if (currentPhase == PHASE_BALL_IN_FIELD) {
        DrawPlayerFigure(fielderRunPos, RED, false, NULL, false);
    }

    DrawPlayerFigure(bowlerAnimPos, RED, false, gameState->bowling_team->name, false);
    DrawText(bowler->name, bowlerAnimPos.x - MeasureText(bowler->name, 10)/2, bowlerAnimPos.y + 30, 10, WHITE);
    
    // Draw ball
    // The ball is only drawn when it's actually in play (traveling or in field)
    if (currentPhase == PHASE_BALL_TRAVEL || currentPhase == PHASE_BALL_IN_FIELD || currentPhase == PHASE_BATSMAN_RUNNING || currentPhase == PHASE_BOUNDARY_ANIMATION) {
        if (ballAltitude > 0) {
            DrawCircle(ballPos.x, ballPos.y, 6, Fade(BLACK, 0.4f)); // Shadow
        }
        DrawCircle(ballPos.x, ballPos.y - ballAltitude, 6, WHITE); // Ball
    }

    // --- Draw Scoreboard on top ---
    char scoreText[100];
    sprintf(scoreText, "%s: %d / %d", gameState->batting_team->name, gameState->total_runs, gameState->wickets);
    DrawText(scoreText, 20, 20, 40, DARKGRAY);

    char oversText[100];
    sprintf(oversText, "Overs: %d.%d / %d", gameState->overs_completed, gameState->balls_bowled_in_over, gameState->max_overs);
    DrawTextBold(oversText, 20, 70, 20, GRAY);
    
    // --- Bottom HUD ---
    char totalScoreText[128];
    if (gameState->target > 0) {
        sprintf(totalScoreText, "Score: %d/%d (Target: %d)", gameState->total_runs, gameState->wickets, gameState->target);
    } else {
        sprintf(totalScoreText, "Score: %d/%d", gameState->total_runs, gameState->wickets);
    }
    DrawTextBold(totalScoreText, 20, GetScreenHeight() - 40, 30, WHITE);

    if (currentPhase == PHASE_IDLE && !isGameOver) {
        DrawText("Press [SPACE] to Bowl Next Ball", 20, GetScreenHeight() - 70, 20, YELLOW);
    } else if (isGameOver) {
        if (gameState->inning_num == 1) {
            currentPhase = PHASE_INNINGS_BREAK;
        } else {
            currentPhase = PHASE_MATCH_OVER;
        }
    }

    // ... (rest of the drawing logic for players, etc.)

    // --- Draw Final Match/Innings Status ---
    if (currentPhase == PHASE_INNINGS_BREAK) {
        DrawText("INNINGS BREAK", GetScreenWidth()/2 - MeasureText("INNINGS BREAK", 40)/2, GetScreenHeight()/2 - 40, 40, YELLOW);
        char targetText[64];
        sprintf(targetText, "Target: %d", gameState->target);
        DrawText(targetText, GetScreenWidth()/2 - MeasureText(targetText, 30)/2, GetScreenHeight()/2 + 10, 30, RAYWHITE);
        DrawText("Press [SPACE] to start 2nd Innings", GetScreenWidth()/2 - MeasureText("Press [SPACE] to start 2nd Innings", 20)/2, GetScreenHeight()/2 + 50, 20, RAYWHITE);
    } else if (currentPhase == PHASE_MATCH_OVER) {
        char winnerText[128];
        if (gameState->total_runs >= gameState->target) {
            sprintf(winnerText, "%s won by %d wickets!", gameState->batting_team->name, 10 - gameState->wickets);
        } else if (gameState->total_runs == gameState->target - 1) {
            sprintf(winnerText, "Match Tied!");
        } else {
            sprintf(winnerText, "%s won by %d runs!", gameState->bowling_team->name, gameState->target - gameState->total_runs -1);
        }
        DrawText("MATCH OVER", GetScreenWidth()/2 - MeasureText("MATCH OVER", 50)/2, GetScreenHeight()/2 - 40, 50, GREEN);
        DrawText(winnerText, GetScreenWidth()/2 - MeasureText(winnerText, 30)/2, GetScreenHeight()/2 + 20, 30, RAYWHITE);
    }
    
    // Draw the back button
    DrawRectangleRec(backButton, LIGHTGRAY);
    DrawText("Back to Menu", backButton.x + backButton.width/2 - MeasureText("Back to Menu", 20)/2, backButton.y + 10, 20, BLACK);

    // Draw "Play and Miss!" message if active
    if (GetTime() < playMissMessageEndTime) {
        DrawText("Play and Miss!", GetScreenWidth()/2 - MeasureText("Play and Miss!", 40)/2, GetScreenHeight()/2, 40, ORANGE);
    }
    if (GetTime() < outcomeMessageEndTime) {
        DrawText(outcomeMessage, GetScreenWidth()/2 - MeasureText(outcomeMessage, 40)/2, GetScreenHeight()/2, 40, ORANGE);
    }
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

    // Input boxes for adding/editing and filtering
    static TextBox nameBox, countryBox, yearBox, matchesBox;
    static TextBox searchBox, filterCountryBox, filterMatchesBox, filterYearBox;
    static bool layoutInitialized = false;

    // --- UI element definitions ---
    if (!layoutInitialized) {
        // Add/Edit boxes
        nameBox =      (TextBox){ { 80, 100, 120, 30 }, {0}, 0, false, false };
        countryBox =   (TextBox){ { 210, 100, 120, 30 }, {0}, 0, false, false };
        yearBox =      (TextBox){ { 340, 100, 120, 30 }, {0}, 0, false, false };
        matchesBox =   (TextBox){ { 470, 100, 120, 30 }, {0}, 0, false, false };

        // Filter boxes
        searchBox =    (TextBox){ { GetScreenWidth() - 640, 60, 200, 25 }, {0}, 0, false, false };
        filterCountryBox = (TextBox){ { GetScreenWidth() - 430, 60, 100, 25 }, {0}, 0, false, false };
        filterMatchesBox = (TextBox){ { GetScreenWidth() - 320, 60, 100, 25 }, {0}, 0, false, false };
        filterYearBox = (TextBox){ { GetScreenWidth() - 210, 60, 100, 25 }, {0}, 0, false, false };
        
        layoutInitialized = true;
    }
    const Rectangle addButton = { 610, 100, 170, 30 };
    const Rectangle backButton = { 20, GetScreenHeight() - 50, 150, 40 };
    const Rectangle cancelEditButton = { 610, 135, 170, 25 };
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
        DrawTextBold("No umpires found in Data/umpires.dat.", view.x + 20, view.y + 10, 20, GRAY); // No scroll needed
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

        Team* selectedTeam = &teams[selectedTeamIndex];
        // --- Player List Refactored with Scrollbars ---
        // --- DYNAMIC COLUMN WIDTH CALCULATION ---
        const char* col_headers[] = { "Name", "Role", "Bat", "Bowl", "WK", "Active", "Bat Skl", "Bwl Skl", "Fld Skl", "M", "Runs", "Wkts", "Stumps", "RunOuts", "Actions" };
        const int num_cols = sizeof(col_headers) / sizeof(col_headers[0]);
        float col_widths[num_cols];

        // 1. Initialize with header widths
        for (int i = 0; i < num_cols; i++) {
            col_widths[i] = MeasureText(col_headers[i], 20) + 10; // Add some padding
        }
        col_widths[num_cols - 1] = 120; // Actions column is fixed

        // 2. Iterate through players to find max widths
        for (int i = 0; i < selectedTeam->num_players; i++) {
            Player* p = &selectedTeam->players[i];
            char buffer[256];

            // Player Name (with suffixes)
            char name_suffix[16] = {0};
            if (p->is_wicketkeeper) strcat(name_suffix, " (WK)");
            if (i == selectedTeam->captain_idx) strcat(name_suffix, " (C)");
            if (i == selectedTeam->vice_captain_idx) strcat(name_suffix, " (VC)");
            snprintf(buffer, sizeof(buffer), "%s%s", p->name, name_suffix);
            if (MeasureText(buffer, 20) > col_widths[0]) col_widths[0] = MeasureText(buffer, 20);

            // Role
            if (MeasureText(playerTypeNames[p->type], 20) > col_widths[1]) col_widths[1] = MeasureText(playerTypeNames[p->type], 20);
            // Batting Style
            if (MeasureText(p->batting_style == BATTING_STYLE_RHB ? "RHB" : "LHB", 20) > col_widths[2]) col_widths[2] = MeasureText(p->batting_style == BATTING_STYLE_RHB ? "RHB" : "LHB", 20);
            // Bowling Style
            if (MeasureText(bowlStyleNames[p->bowling_style], 20) > col_widths[3]) col_widths[3] = MeasureText(bowlStyleNames[p->bowling_style], 20);
            // WK & Active (Yes/No)
            if (MeasureText("Yes", 20) > col_widths[4]) col_widths[4] = MeasureText("Yes", 20);
            if (MeasureText("Yes", 20) > col_widths[5]) col_widths[5] = MeasureText("Yes", 20);

            // Skills
            if (MeasureText(TextFormat("%d", p->batting_skill), 20) > col_widths[6]) col_widths[6] = MeasureText(TextFormat("%d", p->batting_skill), 20);
            if (MeasureText(TextFormat("%d", p->bowling_skill), 20) > col_widths[7]) col_widths[7] = MeasureText(TextFormat("%d", p->bowling_skill), 20);
            if (MeasureText(TextFormat("%d", p->fielding_skill), 20) > col_widths[8]) col_widths[8] = MeasureText(TextFormat("%d", p->fielding_skill), 20);

            // Stats
            if (MeasureText(TextFormat("%d", p->matches_played), 20) > col_widths[9]) col_widths[9] = MeasureText(TextFormat("%d", p->matches_played), 20);
            if (MeasureText(TextFormat("%d", p->total_runs), 20) > col_widths[10]) col_widths[10] = MeasureText(TextFormat("%d", p->total_runs), 20);
            if (MeasureText(TextFormat("%d", p->total_wickets), 20) > col_widths[11]) col_widths[11] = MeasureText(TextFormat("%d", p->total_wickets), 20);
            if (MeasureText(TextFormat("%d", p->total_stumpings), 20) > col_widths[12]) col_widths[12] = MeasureText(TextFormat("%d", p->total_stumpings), 20);
            if (MeasureText(TextFormat("%d", p->total_run_outs), 20) > col_widths[13]) col_widths[13] = MeasureText(TextFormat("%d", p->total_run_outs), 20);
        }

        const int col_padding = 25; // Increased padding
        
        float contentWidth = 20; // Initial left padding
        for (int i = 0; i < num_cols; i++) {
            contentWidth += col_widths[i] + col_padding;
        }
        float contentHeight = 40 + (selectedTeam->num_players * 25); // 40 for header

        Rectangle playerListView = { panelRight.x + 1, panelRight.y + 50, panelRight.width - 2, panelRight.height - 60 };
        
        Rectangle view = playerListView; // The area where content is drawn
        DrawRectangleRec(view, WHITE); // Draw background for the whole area

        bool horizScrollbarRequired = contentWidth > view.width;
        bool vertScrollbarRequired = contentHeight > view.height;

        // Reserve space for scrollbars if they are needed, this also defines the scissor rect
        Rectangle scissorView = playerListView;
        if (horizScrollbarRequired) scissorView.height -= 12;
        if (vertScrollbarRequired) scissorView.width -= 12;

        // Clamp Scrolling Values based on the actual drawable area (scissorView)
        if (playerScroll.y > 0) playerScroll.y = 0;
        if (vertScrollbarRequired && playerScroll.y < scissorView.height - contentHeight) playerScroll.y = scissorView.height - contentHeight;
        if (!vertScrollbarRequired) playerScroll.y = 0;

        if (playerScroll.x > 0) playerScroll.x = 0;
        if (horizScrollbarRequired && playerScroll.x < scissorView.width - contentWidth) playerScroll.x = scissorView.width - contentWidth;
        if (!horizScrollbarRequired) playerScroll.x = 0;

        // Handle mouse wheel scrolling
        if (CheckCollisionPointRec(GetMousePosition(), scissorView)) {
            playerScroll.y += GetMouseWheelMove() * 25.0f;
            if (IsKeyDown(KEY_LEFT_SHIFT)) {
                 playerScroll.x += GetMouseWheelMove() * 25.0f;
            }
        }
        
        BeginScissorMode(scissorView.x, scissorView.y, scissorView.width, scissorView.height);

        // Draw Headers
        float col_x[num_cols];
        col_x[0] = scissorView.x + 10 + playerScroll.x;
        for (int i = 1; i < num_cols; i++) {
            col_x[i] = col_x[i-1] + col_widths[i-1] + col_padding;
        }
        for (int i = 0; i < num_cols; i++) {
            DrawTextBold(col_headers[i], col_x[i], scissorView.y + 10, 20, DARKGRAY);
        }

        // Draw Players
        for (int i = 0; i < selectedTeam->num_players; i++) {
            Player* p = &selectedTeam->players[i];
            float y_pos = scissorView.y + 40 + (i * 25) + playerScroll.y;
             if (y_pos < scissorView.y - 25 || y_pos > scissorView.y + scissorView.height) continue; // Culling
            
            Color textColor = BLACK;
            char name_suffix[16] = {0};

            if (p->is_wicketkeeper) {
                strcat(name_suffix, " (WK)");
            }
            if (i == selectedTeam->captain_idx) {
                textColor = GOLD;
                strcat(name_suffix, " (C)");
            } else if (i == selectedTeam->vice_captain_idx) {
                textColor = SILVER;
                strcat(name_suffix, " (VC)");
            }

            char player_display_name[MAX_PLAYER_NAME_LEN + 16];
            snprintf(player_display_name, sizeof(player_display_name), "%s%s", p->name, name_suffix);

            DrawText(player_display_name, col_x[0], y_pos, 20, textColor);
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

            Rectangle editBtn = { col_x[14], y_pos, 50, 20 };
            Rectangle deleteBtn = { col_x[14] + 55, y_pos, 60, 20 };
            DrawRectangleRec(editBtn, ORANGE); DrawText("Edit", editBtn.x + 10, editBtn.y + 2, 15, BLACK);
            DrawRectangleRec(deleteBtn, RED); DrawText("Delete", deleteBtn.x + 5, deleteBtn.y + 2, 15, WHITE);

            if (CheckCollisionPointRec(GetMousePosition(), editBtn) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                editPlayerIndex = i;
                showAddPlayerForm = true;
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
                for (int j = i; j < selectedTeam->num_players - 1; j++) {
                    selectedTeam->players[j] = selectedTeam->players[j + 1];
                }
                selectedTeam->num_players--;
                save_teams(teams, num_teams);
                needs_refresh = true;
                break;
            }
        }
        
        EndScissorMode();
        
        // --- Draw and Handle Vertical Scrollbar ---
        if (vertScrollbarRequired) {
            Rectangle scrollBarArea = { scissorView.x + scissorView.width, scissorView.y, 10, scissorView.height };
            DrawRectangleRec(scrollBarArea, LIGHTGRAY);
            
            float handleHeight = (scissorView.height / contentHeight) * scissorView.height;
            if (handleHeight < 20) handleHeight = 20; // Minimum handle size
            float handleY = scissorView.y + (-playerScroll.y / (contentHeight - scissorView.height)) * (scissorView.height - handleHeight);
            Rectangle scrollHandle = { scrollBarArea.x, handleY, 10, handleHeight };
            DrawRectangleRec(scrollHandle, DARKGRAY);

            static bool isDragging = false;
            if (CheckCollisionPointRec(GetMousePosition(), scrollHandle) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) isDragging = true;
            if (isDragging) {
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) isDragging = false;
                else {
                    // Relative mouse movement
                    playerScroll.y -= GetMouseDelta().y * (contentHeight / scissorView.height);
                }
            }
        }

        // --- Draw and Handle Horizontal Scrollbar ---
        if (horizScrollbarRequired) {
            Rectangle scrollBarArea = { scissorView.x, scissorView.y + scissorView.height, scissorView.width, 10 };
            DrawRectangleRec(scrollBarArea, LIGHTGRAY);
            
            float handleWidth = (scissorView.width / contentWidth) * scissorView.width;
            if (handleWidth < 20) handleWidth = 20; // Minimum handle size
            float handleX = scissorView.x + (-playerScroll.x / (contentWidth - scissorView.width)) * (scissorView.width - handleWidth);
            Rectangle scrollHandle = { handleX, scrollBarArea.y, handleWidth, 10 };
            DrawRectangleRec(scrollHandle, DARKGRAY);

            static bool isDragging = false;
            if (CheckCollisionPointRec(GetMousePosition(), scrollHandle) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) isDragging = true;
            if (isDragging) {
                if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) isDragging = false;
                else {
                    // Relative mouse movement
                    playerScroll.x -= GetMouseDelta().x * (contentWidth / scissorView.width);
                }
            }
        }
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

static void UpdateDrawMatchSetupScreen(GuiState *state) {
    const int screenWidth = GetScreenWidth();
    const int screenHeight = GetScreenHeight();
    const char *title = "Select Match Type";

    const char *options[] = {
        "One Day World Cup", "T20 World Cup", "Champions Trophy", "Bilateral Series",
        "Tri-Series", "IPL", "Asia Cup", "The Ashes"
    };
    const int numOptions = sizeof(options) / sizeof(options[0]);
    const int buttonWidth = 400;
    const int buttonHeight = 50;
    const int buttonSpacing = 15;
    const float totalHeight = (numOptions * (buttonHeight + buttonSpacing)) - buttonSpacing;
    float startY = (screenHeight - totalHeight) / 2.0f;

    const Rectangle backButton = { 20, screenHeight - 60, 150, 40 };

    BeginDrawing();
    ClearBackground(RAYWHITE);
    DrawTextBold(title, screenWidth / 2 - MeasureText(title, 40) / 2, startY - 80, 40, DARKGRAY);

    for (int i = 0; i < numOptions; i++) {
        Rectangle buttonRect = { screenWidth / 2.0f - buttonWidth / 2.0f, startY + i * (buttonHeight + buttonSpacing), buttonWidth, buttonHeight };
        bool hovered = CheckCollisionPointRec(GetMousePosition(), buttonRect);

        DrawRectangleRec(buttonRect, hovered ? SKYBLUE : LIGHTGRAY);
        DrawText(options[i], buttonRect.x + buttonRect.width / 2 - MeasureText(options[i], 20) / 2, buttonRect.y + 15, 20, BLACK);

        if (hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (strcmp(options[i], "One Day World Cup") == 0) {
                ChangeScreen(state, SCREEN_WC_SETUP);
            } else {
                // For now, all other buttons lead to a placeholder.
                ChangeScreen(state, SCREEN_PLACEHOLDER);
            }
        }
    }


    // Draw Back Button
    DrawRectangleRec(backButton, MAROON);
    DrawTextBold("Back", backButton.x + backButton.width/2 - MeasureText("Back", 20)/2, backButton.y + 10, 20, WHITE);
    if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) ChangeScreen(state, state->previousScreen);

    EndDrawing();
}

static Player generate_random_player_with_role(PlayerType type) {
    Player p = {0};
    const char* prefixes[] = {"R.", "S.", "K.", "M."};
    const char* names[] = {"Singh", "Kumar", "Patel", "Sharma", "Khan", "Reddy"};
    sprintf(p.name, "%s %s (Gen)", prefixes[rand() % 4], names[rand() % 6]);

    p.type = type;
    p.is_active = true;
    p.batting_style = (rand() % 2 == 0) ? BATTING_STYLE_RHB : BATTING_STYLE_LHB;

    switch(type) {
        case PLAYER_TYPE_BATSMAN:
            p.batting_skill = 60 + (rand() % 30); // 60-89
            p.bowling_skill = 10 + (rand() % 20); // 10-29
            p.bowling_style = BOWLING_STYLE_NONE;
            break;
        case PLAYER_TYPE_BOWLER:
            p.batting_skill = 20 + (rand() % 30); // 20-49
            p.bowling_skill = 60 + (rand() % 30); // 60-89
            p.bowling_style = (BowlingStyle)(BOWLING_STYLE_RF + (rand() % 10)); // Random real bowling style
            break;
        case PLAYER_TYPE_WICKETKEEPER:
            p.is_wicketkeeper = true;
            p.batting_skill = 50 + (rand() % 30); // 50-79
            p.bowling_skill = 10 + (rand() % 10); // 10-19
            p.bowling_style = BOWLING_STYLE_NONE;
            break;
        case PLAYER_TYPE_ALLROUNDER: // Fallback, can be used if needed
            p.batting_skill = 55 + (rand() % 25); // 55-79
            p.bowling_skill = 55 + (rand() % 25); // 55-79
            p.bowling_style = (BowlingStyle)(BOWLING_STYLE_RF + (rand() % 10));
            break;
    }
    return p;
}

typedef enum {
    WC_STEP_TEAM_SELECTION,
    WC_STEP_SQUAD_SELECTION,
    WC_STEP_USER_TEAM_CHOICE,
    WC_STEP_FIXTURES,
    WC_STEP_PRE_MATCH
} WorldCupSetupStep;

static void UpdateDrawWcSetupScreen(GuiState *state, GameState *gameState, GameSounds *sounds) {
    // --- Screen State ---
    static WorldCupSetupStep currentStep = WC_STEP_TEAM_SELECTION;
    static Team* all_teams = NULL;
    static int num_all_teams = 0;
    static bool* selected_teams_mask = NULL;
    static bool needs_refresh = true;
    
    // State for subsequent steps
    static Team* wc_teams = NULL; // The teams actually in the world cup
    static int num_wc_teams = 0;
    static bool player_selection_mask[MAX_PLAYERS];
    static int user_team_idx = -1;
    static char validation_error[128] = {0};
    
    // State for the two-step playing XI selection
    static int squad_selection_turn = 0; // 0 for user's team, 1 for opponent's team
    static int designated_wk_idx = -1; // -1 for no designated wicketkeeper
    static Team match_team_A, match_team_B; // Temporary teams to hold the final 11 players for the match


    static Match wc_matches[MAX_MATCHES_IN_TOURNAMENT];
    static int current_match_idx = 0;
    static int num_wc_matches = 0;

    // State for pre-match setup
    static time_t tournament_start_date;
    static float rain_percentage = 0.0f;


    // State for Toss
    static bool toss_in_progress = false;
    static float toss_rotation = 0.0f;
    static float toss_velocity = 3600.0f;
    static int toss_call = -1; // 0 for Heads, 1 for Tails
    static double toss_end_time = 0.0;
    static int toss_result = -1;

    static Vector2 scroll = {0, 0};

    const Rectangle backButton = { 20, GetScreenHeight() - 60, 150, 40 };

    // --- Logic ---
    if (needs_refresh) {
        if (all_teams) free(all_teams);
        if (selected_teams_mask) free(selected_teams_mask);
        if (wc_teams) { free(wc_teams); wc_teams = NULL; }

        all_teams = load_teams(&num_all_teams);
        if (num_all_teams > 0) {
            selected_teams_mask = calloc(num_all_teams, sizeof(bool));
        }
        needs_refresh = false;
        currentStep = WC_STEP_TEAM_SELECTION; // Always start at team selection
        num_wc_teams = 0;
        user_team_idx = -1;
        validation_error[0] = '\0';
        num_wc_matches = 0;
        current_match_idx = 0;
        toss_call = -1; toss_result = -1; toss_end_time = 0.0;
        squad_selection_turn = 0;
    }

    if (CheckCollisionPointRec(GetMousePosition(), backButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (IsSoundPlaying(sounds->toss)) StopSound(sounds->toss);
        ChangeScreen(state, SCREEN_MATCH_SETUP);
        needs_refresh = true; // Force a full refresh next time we enter this screen
    }

    // --- Drawing ---
    BeginDrawing();
    ClearBackground(RAYWHITE);

    switch (currentStep) {
        case WC_STEP_TEAM_SELECTION: {
            DrawTextBold("World Cup Setup: Select Teams", GetScreenWidth() / 2 - MeasureText("World Cup Setup: Select Teams", 40) / 2, 20, 40, DARKGRAY);
            DrawText("Choose the teams that will participate in the tournament.", GetScreenWidth() / 2 - MeasureText("Choose the teams that will participate in the tournament.", 20) / 2, 70, 20, GRAY);

            if (num_all_teams == 0) {
                DrawText("No teams available. Please create teams first.", 50, 150, 20, RED);
            } else {
                const int buttonWidth = 300;
                const int buttonHeight = 40;
                const int columns = (GetScreenWidth() - 100) / (buttonWidth + 20);
                int count = 0;

                for (int i = 0; i < num_all_teams; i++) {
                    if (all_teams[i].is_deleted || all_teams[i].is_hidden) continue;

                    int col = count % columns;
                    int row = count / columns;
                    Rectangle teamButton = { 50 + col * (buttonWidth + 20), 120 + row * (buttonHeight + 10), buttonWidth, buttonHeight };
                    
                    bool isSelected = selected_teams_mask[i];
                    DrawRectangleRec(teamButton, isSelected ? DARKBLUE : LIGHTGRAY);
                    DrawText(all_teams[i].name, teamButton.x + 15, teamButton.y + 10, 20, isSelected ? WHITE : BLACK);

                    if (CheckCollisionPointRec(GetMousePosition(), teamButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        selected_teams_mask[i] = !selected_teams_mask[i];
                    }
                    count++;
                }

                float buttonRowY = GetScreenHeight() - 120; // Moved buttons up to avoid overlap
                float totalButtonsWidth = 150 + 150 + 10; // width of both buttons plus spacing
                float startX = (GetScreenWidth() - totalButtonsWidth) / 2;

                Rectangle selectAllButton = { startX, buttonRowY, 150, 40 };
                DrawRectangleRec(selectAllButton, DARKBLUE);
                DrawTextBold("Select All", selectAllButton.x + 35, selectAllButton.y + 10, 20, WHITE);
                if (CheckCollisionPointRec(GetMousePosition(), selectAllButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    for (int i = 0; i < num_all_teams; i++) {
                        if (!all_teams[i].is_deleted && !all_teams[i].is_hidden) {
                            selected_teams_mask[i] = true;
                        }
                    }
                }

                Rectangle deselectAllButton = { startX + 160, buttonRowY, 150, 40 };
                DrawRectangleRec(deselectAllButton, MAROON);
                DrawTextBold("Deselect All", deselectAllButton.x + 25, deselectAllButton.y + 10, 20, WHITE);
                if (CheckCollisionPointRec(GetMousePosition(), deselectAllButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    for (int i = 0; i < num_all_teams; i++) {
                        selected_teams_mask[i] = false;
                    }
                }

                // "Next" button to proceed to squad selection
                Rectangle nextButton = { GetScreenWidth() - 200, buttonRowY, 150, 40 };
                DrawRectangleRec(nextButton, DARKGREEN);
                DrawTextBold("Next", nextButton.x + 50, nextButton.y + 10, 20, WHITE);
                if (CheckCollisionPointRec(GetMousePosition(), nextButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    // Count selected teams and prepare for squad selection
                    num_wc_teams = 0;
                    for (int i = 0; i < num_all_teams; i++) {
                        if (selected_teams_mask[i]) num_wc_teams++;
                    }

                    if (num_wc_teams >= 2) {
                        if (wc_teams) free(wc_teams);
                        wc_teams = malloc(num_wc_teams * sizeof(Team));
                        int current_wc_team = 0;
                        for (int i = 0; i < num_all_teams; i++) {
                            if (selected_teams_mask[i]) {
                                wc_teams[current_wc_team++] = all_teams[i];
                            }
                        }
                        validation_error[0] = '\0';
                        currentStep = WC_STEP_USER_TEAM_CHOICE; // Go directly to user team choice
                    } else {
                        strcpy(validation_error, "You must select at least 2 teams.");
                    }
                }
                if (validation_error[0] != '\0') {
                    DrawText(validation_error, GetScreenWidth() - MeasureText(validation_error, 20) - 220, GetScreenHeight() - 50, 20, RED);
                }
            }
            break;
        }
        case WC_STEP_USER_TEAM_CHOICE: {
            DrawTextBold("Choose Your Team", GetScreenWidth() / 2 - MeasureText("Choose Your Team", 40) / 2, 20, 40, DARKGRAY);
            DrawText("Select the team you want to play as for the World Cup.", GetScreenWidth() / 2 - MeasureText("Select the team you want to play as for the World Cup.", 20) / 2, 70, 20, GRAY);

            const int buttonWidth = 300;
            const int buttonHeight = 40;
            const int columns = (GetScreenWidth() - 100) / (buttonWidth + 20);

            for (int i = 0; i < num_wc_teams; i++) {
                int col = i % columns;
                int row = i / columns;
                Rectangle teamButton = { 50 + col * (buttonWidth + 20), 120 + row * (buttonHeight + 10), buttonWidth, buttonHeight };
                DrawRectangleRec(teamButton, LIGHTGRAY);
                DrawText(wc_teams[i].name, teamButton.x + 15, teamButton.y + 10, 20, BLACK);

                if (CheckCollisionPointRec(GetMousePosition(), teamButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    user_team_idx = i;

                    // --- Intelligent Auto-Completion of Teams ---
                    for (int team_idx = 0; team_idx < num_wc_teams; team_idx++) {
                        Team* current_team = &wc_teams[team_idx];
                        if (current_team->num_players >= 11) continue;

                        printf("Team '%s' has %d players. Auto-completing to 11.\n", current_team->name, current_team->num_players);

                        int wk_count = 0;
                        int bowler_count = 0;
                        for (int p_idx = 0; p_idx < current_team->num_players; p_idx++) {
                            if (current_team->players[p_idx].is_wicketkeeper) wk_count++;
                            if (current_team->players[p_idx].type == PLAYER_TYPE_BOWLER || current_team->players[p_idx].type == PLAYER_TYPE_ALLROUNDER) {
                                bowler_count++;
                            }
                        }

                        int wk_needed = (wk_count < 1) ? 1 - wk_count : 0;
                        int bowlers_needed = (bowler_count < 5) ? 5 - bowler_count : 0;

                        while (current_team->num_players < 11) {
                            Player new_player;
                            if (wk_needed > 0) {
                                new_player = generate_random_player_with_role(PLAYER_TYPE_WICKETKEEPER);
                                wk_needed--;
                                printf(" -> Added Wicketkeeper: %s\n", new_player.name);
                            } else if (bowlers_needed > 0) {
                                new_player = generate_random_player_with_role(PLAYER_TYPE_BOWLER);
                                bowlers_needed--;
                                printf(" -> Added Bowler: %s\n", new_player.name);
                            } else {
                                new_player = generate_random_player_with_role(PLAYER_TYPE_BATSMAN);
                                printf(" -> Added Batsman: %s\n", new_player.name);
                            }

                            if (current_team->num_players < MAX_PLAYERS) {
                                current_team->players[current_team->num_players++] = new_player;
                            }
                        }
                    }
                    // --- End of auto-fill logic ---


                    // Generate fixtures
                    num_wc_matches = 0;
                    for (int team_i = 0; team_i < num_wc_teams; team_i++) {
                        for (int team_j = team_i + 1; team_j < num_wc_teams; team_j++) {
                            if (num_wc_matches < MAX_MATCHES_IN_TOURNAMENT) {
                                wc_matches[num_wc_matches].teamA = &wc_teams[team_i];
                                wc_matches[num_wc_matches].teamB = &wc_teams[team_j];
                                wc_matches[num_wc_matches].format = FORMAT_ODI;
                                num_wc_matches++;
                            }
                        }
                    }
                    currentStep = WC_STEP_FIXTURES;
                }
            }
            break;
        }
        case WC_STEP_FIXTURES: {
            DrawTextBold("Tournament Fixtures", GetScreenWidth() / 2 - MeasureText("Tournament Fixtures", 40) / 2, 20, 40, DARKGRAY);
            DrawText("All teams will play each other once.", GetScreenWidth() / 2 - MeasureText("All teams will play each other once.", 20) / 2, 70, 20, GRAY);

            Rectangle view = { 50, 110, GetScreenWidth() - 100, GetScreenHeight() - 220 };
            DrawRectangleLinesEx(view, 1, DARKGRAY);

            const float itemHeight = 30.0f;
            if (CheckCollisionPointRec(GetMousePosition(), view)) {
                scroll.y += GetMouseWheelMove() * itemHeight;
                if (scroll.y > 0) scroll.y = 0;
            }

            BeginScissorMode(view.x, view.y, view.width, view.height);
            for (int i = 0; i < num_wc_matches; i++) {
                float y_pos = view.y + 10 + (i * itemHeight) + scroll.y;
                
                // Add Date and Time to fixtures
                time_t match_date_t = tournament_start_date + (i * 24 * 60 * 60);
                struct tm *match_tm = localtime(&match_date_t);
                char date_str[32];
                strftime(date_str, sizeof(date_str), "%a, %b %d", match_tm);
                char match_text[512];
                sprintf(match_text, "Match %d: %s vs %s  |  %s @ 10:00 AM", i + 1, wc_matches[i].teamA->name, wc_matches[i].teamB->name, date_str);
                DrawText(match_text, view.x + 20, y_pos, 20, DARKGRAY);
            }
            EndScissorMode();

            Rectangle startButton = { GetScreenWidth() - 220, GetScreenHeight() - 60, 200, 40 };
            DrawRectangleRec(startButton, DARKGREEN);
            DrawTextBold("Start Tournament", startButton.x + 20, startButton.y + 10, 20, WHITE);
            if (CheckCollisionPointRec(GetMousePosition(), startButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                current_match_idx = 0;
                tournament_start_date = time(NULL); // Set start date to today
                // TODO: Here we would start iterating through matches. For now, we go to pre-match for the first user match.
                currentStep = WC_STEP_PRE_MATCH;
            }
            break;
        }
        case WC_STEP_PRE_MATCH: {
            Match* currentMatch = &wc_matches[current_match_idx];
            char title[256];
            
            // Calculate match date
            time_t match_date_t = tournament_start_date + (current_match_idx * 24 * 60 * 60); // Add days
            struct tm *match_tm = localtime(&match_date_t);
            char date_str[64];
            strftime(date_str, sizeof(date_str), "%A, %B %d, %Y", match_tm);

            sprintf(title, "%s vs %s", currentMatch->teamA->name, currentMatch->teamB->name);
            DrawTextBold(title, GetScreenWidth() / 2 - MeasureText(title, 40) / 2, 20, 40, DARKGRAY);
            DrawText(date_str, GetScreenWidth() / 2 - MeasureText(date_str, 20) / 2, 70, 20, GRAY);
            
            // --- Distinguish between User and AI matches ---
            Team* user_team = &wc_teams[user_team_idx];
            bool is_user_match = (currentMatch->teamA == user_team) || (currentMatch->teamB == user_team);

            if (!is_user_match) {
                DrawText("Simulating AI vs AI match...", GetScreenWidth()/2 - MeasureText("Simulating AI vs AI match...", 30)/2, GetScreenHeight()/2, 30, DARKGRAY);
                
                // Use the toss_end_time as a delay timer before skipping to the next match
                if (toss_end_time == 0) toss_end_time = GetTime() + 1.5; // Initial delay
                if (GetTime() > toss_end_time) {
                    current_match_idx++;
                    if (current_match_idx >= num_wc_matches) {
                        currentStep = WC_STEP_FIXTURES; // Loop back to fixtures for now
                    }
                    // Reset for the next match (which might also be AI)
                    toss_end_time = GetTime() + 1.5; 
                }
                break; // Skip the rest of the pre-match logic for AI matches
            }


            // --- Toss Simulation (For User Matches Only) ---
            if (toss_in_progress) {
                toss_rotation += toss_velocity * GetFrameTime();
                toss_velocity -= 3000.0f * GetFrameTime(); // Decelerate
                if (toss_velocity <= 0) {
                    toss_velocity = 0;
                    toss_in_progress = false;
                    toss_result = (rand() % 2); // 0 for Heads, 1 for Tails
                    toss_end_time = GetTime() + 5.0; // Set time to hold the result for 5 seconds
                }
                DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 100, GOLD);
                DrawTextPro(GetFontDefault(), "H", (Vector2){GetScreenWidth()/2, GetScreenHeight()/2}, (Vector2){50,50}, toss_rotation, 100, 10, BLACK);
            } else if (toss_call == -1) { // Ask for user's call
                if (!IsSoundPlaying(sounds->toss)) PlaySound(sounds->toss);
                // --- Draw the captains and umpire before the toss ---
                Vector2 captainAPos = { GetScreenWidth()/2 - 200, GetScreenHeight()/2 };
                Vector2 captainBPos = { GetScreenWidth()/2 + 200, GetScreenHeight()/2 };
                Vector2 umpirePos = { GetScreenWidth()/2, GetScreenHeight()/2 - 50 };

                DrawPlayerFigure(captainAPos, BLUE, false, currentMatch->teamA->name, false);
                DrawText(currentMatch->teamA->players[0].name, captainAPos.x - MeasureText(currentMatch->teamA->players[0].name, 10)/2, captainAPos.y + 30, 10, BLACK);

                DrawPlayerFigure(captainBPos, RED, false, currentMatch->teamB->name, false);
                DrawText(currentMatch->teamB->players[0].name, captainBPos.x - MeasureText(currentMatch->teamB->players[0].name, 10)/2, captainBPos.y + 30, 10, BLACK);

                DrawPlayerFigure(umpirePos, BLACK, false, "Umpire", false);
// ---
                DrawText("Call the Toss!", GetScreenWidth() / 2 - MeasureText("Call the Toss!", 30) / 2, 200, 30, DARKBLUE);
                Rectangle headsBtn = { GetScreenWidth() / 2 - 210, 250, 200, 50 };
                Rectangle tailsBtn = { GetScreenWidth() / 2 + 10, 250, 200, 50 };
                DrawRectangleRec(headsBtn, LIGHTGRAY); DrawText("HEADS", headsBtn.x + 60, headsBtn.y + 15, 20, BLACK);
                DrawRectangleRec(tailsBtn, LIGHTGRAY); DrawText("TAILS", tailsBtn.x + 65, tailsBtn.y + 15, 20, BLACK);
                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckCollisionPointRec(GetMousePosition(), headsBtn)) toss_call = 0;
                    if (CheckCollisionPointRec(GetMousePosition(), tailsBtn)) toss_call = 1;
                    if (toss_call != -1) {
                        toss_in_progress = true;
                        toss_velocity = 2000.0f + (rand() % 1500); // Randomize spin speed
                    }
                }
            } else { // Show toss result and other options
                char toss_result_text[128];
                bool user_won_toss = (toss_call == toss_result);
                static int toss_choice = -1; // 0 for bat, 1 for field

                sprintf(toss_result_text, "It's %s. You %s the toss!", toss_result == 0 ? "Heads" : "Tails", user_won_toss ? "WON" : "LOST");
                DrawText(toss_result_text, GetScreenWidth() / 2 - MeasureText(toss_result_text, 30) / 2, 120, 30, user_won_toss ? DARKGREEN : MAROON);

                // --- Draw the stationary coin showing the correct result ---
                const char* coin_face = (toss_result == 0) ? "H" : "T";
                DrawCircle(GetScreenWidth() / 2, GetScreenHeight() / 2, 100, GOLD);
                DrawText(coin_face, GetScreenWidth() / 2 - MeasureText(coin_face, 100)/2, GetScreenHeight() / 2 - 50, 100, BLACK);

                if (user_won_toss) {
                    if (toss_choice == -1) {
                        DrawText("What will you do?", GetScreenWidth() / 2 - MeasureText("What will you do?", 20) / 2, 300, 20, BLACK);
                        Rectangle batBtn = { GetScreenWidth() / 2 - 155, 330, 150, 50 };
                        Rectangle fieldBtn = { GetScreenWidth() / 2 + 5, 330, 150, 50 };
                        DrawRectangleRec(batBtn, LIGHTGRAY); DrawText("BAT", batBtn.x + 55, batBtn.y + 15, 20, BLACK);
                        DrawRectangleRec(fieldBtn, LIGHTGRAY); DrawText("FIELD", fieldBtn.x + 50, fieldBtn.y + 15, 20, BLACK);

                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            if (CheckCollisionPointRec(GetMousePosition(), batBtn)) {
                                toss_choice = 0;
                            }
                            if (CheckCollisionPointRec(GetMousePosition(), fieldBtn)) {
                                toss_choice = 1;
                            }
                        }
                    }
                } else { // User lost the toss
                    if (toss_choice == -1) {
                        toss_choice = rand() % 2; // AI makes a choice
                    }
                }

                if (toss_choice != -1) {
                    char choice_text[128];
                    if (user_won_toss) {
                        sprintf(choice_text, "You chose to %s first.", toss_choice == 0 ? "BAT" : "FIELD");
                    } else {
                        sprintf(choice_text, "Opponent chose to %s first.", toss_choice == 0 ? "BAT" : "FIELD");
                    }
                    DrawText(choice_text, GetScreenWidth() / 2 - MeasureText(choice_text, 20) / 2, GetScreenHeight() - 100, 20, DARKBLUE);
                    
                    // --- Proceed Button ---
                    Rectangle proceedButton = { GetScreenWidth() - 220, GetScreenHeight() - 60, 200, 40 };
                    DrawRectangleRec(proceedButton, DARKGREEN);
                    DrawTextBold("Select Playing XI", proceedButton.x + 15, proceedButton.y + 10, 20, WHITE);
                    if (CheckCollisionPointRec(GetMousePosition(), proceedButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        if (IsSoundPlaying(sounds->toss)) StopSound(sounds->toss);
                        // Transition to the player selection screen for this match
                        currentStep = WC_STEP_SQUAD_SELECTION;
                        squad_selection_turn = 0; // Start with the user's team
                        memset(player_selection_mask, 0, sizeof(player_selection_mask)); // Clear selection mask
                        validation_error[0] = '\0'; // Clear any previous errors
                        
                        // Set batting and fielding teams based on toss choice
                        Team* opponent_team = (currentMatch->teamA == user_team) ? currentMatch->teamB : currentMatch->teamA;
                        bool user_bats_first = (user_won_toss && toss_choice == 0) || (!user_won_toss && toss_choice == 1);
                        
                        if (user_bats_first) {
                            match_team_A = *user_team;
                            match_team_B = *opponent_team;
                        } else {
                            match_team_A = *opponent_team;
                            match_team_B = *user_team;
                        }
                        
                        toss_call = -1; toss_result = -1; toss_choice = -1; // Reset toss state for the next match
                    }
                }
            }
            break;
        }
        case WC_STEP_SQUAD_SELECTION: {
            // This step is now used for pre-match Playing XI selection.
            Match* currentMatch = &wc_matches[current_match_idx];
            Team* user_team = &wc_teams[user_team_idx];
            Team* opponent_team = (currentMatch->teamA == user_team) ? currentMatch->teamB : currentMatch->teamA;
            
            Team* team_to_select_for;
            if (squad_selection_turn == 0) {
                team_to_select_for = user_team;
            } else {
                team_to_select_for = opponent_team;
            }

            // Autoplay logic for non-user matches
             if (currentMatch->teamA != user_team && currentMatch->teamB != user_team) {
                 // This is not the user's match, so it should be autoplayed.
                 // For now, we'll just go to a placeholder.
                 // TODO: Implement autoplay logic for non-user matches.
                 DrawText("AUTOPLAYING MATCH...", 100, 100, 30, DARKGRAY);
                 // In a real scenario, you'd simulate the match here and move to the next one.
                 // For now, we just get stuck on a placeholder.
                 break;
             }

            char title[256];
            sprintf(title, "Select Playing XI for %s", team_to_select_for->name);
            DrawTextBold(title, GetScreenWidth() / 2 - MeasureText(title, 40) / 2, 20, 40, DARKGRAY);

            int selected_count = 0;
            Rectangle defaultXIButton = { GetScreenWidth() - 230, 75, 200, 30 };
            DrawRectangleRec(defaultXIButton, VIOLET);
            DrawTextBold("Select Default XI", defaultXIButton.x + 20, defaultXIButton.y + 5, 20, WHITE);


            for(int i=0; i<team_to_select_for->num_players; i++) if(player_selection_mask[i]) selected_count++;
            DrawText(TextFormat("Selected: %d/11", selected_count), 50, 80, 20, selected_count == 11 ? DARKGREEN : BLACK);

            // Player list
            Rectangle view = { 50, 110, GetScreenWidth() - 100, GetScreenHeight() - 220 };
            DrawRectangleLinesEx(view, 1, DARKGRAY);

            const float itemHeight = 30.0f;
            if (CheckCollisionPointRec(GetMousePosition(), view)) {
                scroll.y += GetMouseWheelMove() * itemHeight;
                if (scroll.y > 0) scroll.y = 0;
            }

            BeginScissorMode(view.x, view.y, view.width, view.height);
            for (int i = 0; i < team_to_select_for->num_players; i++) {
                Rectangle playerButton = { view.x + 10, view.y + 10 + (i * itemHeight) + scroll.y, view.width - 200, itemHeight - 2 };
                bool isSelected = player_selection_mask[i];
                DrawRectangleRec(playerButton, isSelected ? SKYBLUE : LIGHTGRAY);
                
                char player_text[256];
                sprintf(player_text, "%s", team_to_select_for->players[i].name);
                if (i == designated_wk_idx) strcat(player_text, " (WK)");
                if (i == team_to_select_for->captain_idx) strcat(player_text, " (C)");
                if (i == team_to_select_for->vice_captain_idx) strcat(player_text, " (VC)");
                DrawText(player_text, playerButton.x + 10, playerButton.y + 5, 20, BLACK);

                if (CheckCollisionPointRec(GetMousePosition(), playerButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    player_selection_mask[i] = !player_selection_mask[i];
                }

                float buttonX = playerButton.x + playerButton.width + 5;

                if (team_to_select_for->players[i].is_wicketkeeper) {
                    Rectangle wk_button = { buttonX, playerButton.y, 40, itemHeight - 2 };
                    DrawRectangleRec(wk_button, (i == designated_wk_idx) ? GREEN : LIGHTGRAY);
                    DrawText("WK", wk_button.x + 10, wk_button.y + 5, 20, BLACK);
                    if (CheckCollisionPointRec(GetMousePosition(), wk_button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        designated_wk_idx = i;
                    }
                    buttonX += 45;
                }

                Rectangle c_button = { buttonX, playerButton.y, 40, itemHeight - 2 };
                Rectangle vc_button = { c_button.x + c_button.width + 5, playerButton.y, 40, itemHeight - 2 };

                DrawRectangleRec(c_button, (i == team_to_select_for->captain_idx) ? GOLD : LIGHTGRAY);
                DrawText("C", c_button.x + 15, c_button.y + 5, 20, BLACK);
                DrawRectangleRec(vc_button, (i == team_to_select_for->vice_captain_idx) ? SILVER : LIGHTGRAY);
                DrawText("VC", vc_button.x + 10, vc_button.y + 5, 20, BLACK);

                if (CheckCollisionPointRec(GetMousePosition(), c_button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (i != team_to_select_for->vice_captain_idx) {
                        team_to_select_for->captain_idx = i;
                    }
                }

                if (CheckCollisionPointRec(GetMousePosition(), vc_button) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (i != team_to_select_for->captain_idx) {
                        team_to_select_for->vice_captain_idx = i;
                    }
                }
            }
            EndScissorMode();

            // Handle "Select Default XI" button click
            if (CheckCollisionPointRec(GetMousePosition(), defaultXIButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                memset(player_selection_mask, 0, sizeof(player_selection_mask)); // Clear current selection

                int wk_needed = 1;
                int bowlers_needed = 5;
                int total_selected = 0;

                // Prioritize pure roles first
                // 1. Select Wicketkeeper(s)
                for (int i = 0; i < team_to_select_for->num_players && wk_needed > 0; i++) {
                    if (team_to_select_for->players[i].is_wicketkeeper) {
                        player_selection_mask[i] = true;
                        wk_needed = 0; // Only need one
                        total_selected++;
                    }
                }
                // 2. Select Bowlers
                for (int i = 0; i < team_to_select_for->num_players && bowlers_needed > 0; i++) {
                    if (!player_selection_mask[i] && (team_to_select_for->players[i].type == PLAYER_TYPE_BOWLER || team_to_select_for->players[i].type == PLAYER_TYPE_ALLROUNDER)) {
                        player_selection_mask[i] = true;
                        bowlers_needed--;
                        total_selected++;
                    }
                }
                // 3. Fill remaining slots with Batsmen
                for (int i = 0; i < team_to_select_for->num_players && total_selected < 11; i++) {
                    if (!player_selection_mask[i]) {
                        player_selection_mask[i] = true;
                        total_selected++;
                    }
                }
            }

            // "Start Match" button
            const char* nextButtonText = (squad_selection_turn == 0) ? "Select Opponent XI" : "Start Match";
            Rectangle nextButton = { GetScreenWidth() - 240, GetScreenHeight() - 60, 220, 40 };
            DrawRectangleRec(nextButton, DARKGREEN);
            DrawTextBold(nextButtonText, nextButton.x + 20, nextButton.y + 10, 20, WHITE);
            if (CheckCollisionPointRec(GetMousePosition(), nextButton) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                // Validation
                int bowlers = 0;
                if (selected_count != 11) {
                    strcpy(validation_error, "You must select exactly 11 players.");
                } else if (designated_wk_idx == -1) {
                    strcpy(validation_error, "You must designate a wicketkeeper.");
                } else if (!player_selection_mask[designated_wk_idx]) {
                    strcpy(validation_error, "The designated wicketkeeper must be in the squad.");
                } else {
                    for(int i=0; i<team_to_select_for->num_players; i++) {
                        if (player_selection_mask[i]) {
                            Player* p = &team_to_select_for->players[i];
                            if (p->type == PLAYER_TYPE_BOWLER || p->type == PLAYER_TYPE_ALLROUNDER) bowlers++;
                        }
                    }
                    if (bowlers < 5) strcpy(validation_error, "Squad must have at least 5 bowling options.");
                    else { // Validation passed
                        validation_error[0] = '\0'; // Clear error

                        if (team_to_select_for->captain_idx == -1) {
                            for (int i = 0; i < team_to_select_for->num_players; i++) {
                                if (player_selection_mask[i]) {
                                    team_to_select_for->captain_idx = i;
                                    break;
                                }
                            }
                        }

                        if (team_to_select_for->vice_captain_idx == -1) {
                            for (int i = 0; i < team_to_select_for->num_players; i++) {
                                if (player_selection_mask[i] && i != team_to_select_for->captain_idx) {
                                    team_to_select_for->vice_captain_idx = i;
                                    break;
                                }
                            }
                        }

                        // Save the selected XI into the appropriate temporary match team
                        Team* temp_match_team = (squad_selection_turn == 0) ? &match_team_A : &match_team_B;
                        *temp_match_team = *team_to_select_for; // Copy team info
                        temp_match_team->num_players = 0; // Reset player count
                        for(int i=0; i<team_to_select_for->num_players; i++) {
                            if(player_selection_mask[i]) {
                                temp_match_team->players[temp_match_team->num_players++] = team_to_select_for->players[i];
                            }
                        }

                        if (squad_selection_turn == 0) {
                            // Move to opponent selection
                            squad_selection_turn = 1;
                            memset(player_selection_mask, 0, sizeof(player_selection_mask)); // Reset for next team
                        } else {
                            // Both teams selected, start the match
                            if (FileExists("Data/saves/resume.dat")) {
                                remove("Data/saves/resume.dat");
                            }
                            
                            // Reset game state for a new match
                            memset(gameState, 0, sizeof(GameState));
                            gameState->batting_team = &match_team_A;
                            gameState->bowling_team = &match_team_B;
                            strcpy(gameState->batting_team_tag, match_team_A.tag);
                            strcpy(gameState->bowling_team_tag, match_team_B.tag);
                            gameState->max_overs = 50; // Set overs for ODI
                            gameState->rain_percentage = rain_percentage; // Pass the rain setting
                            gameState->inning_num = 1;
                            
                            ChangeScreen(state, SCREEN_GAMEPLAY);
                        }
                    }
                }
            }
            if (validation_error[0] != '\0') {
                DrawText(validation_error, nextButton.x - MeasureText(validation_error, 20) - 20, nextButton.y + 10, 20, RED);
            }
            break;
        }
        default:
            break;
    }

    // Draw Back Button on all steps
    DrawRectangleRec(backButton, MAROON);
    DrawTextBold("Back", backButton.x + backButton.width/2 - MeasureText("Back", 20)/2, backButton.y + 10, 20, WHITE);

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
