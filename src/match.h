#ifndef MATCH_H
#define MATCH_H

#include "common.h"
#include "teams.h"
#include <stdbool.h> // For bool type
#include <stdio.h>   // For FILE operations

#include "field_setups.h" // Required for get_field_setup declaration

// Explicitly define NUM_FIELDERS here for GameState struct definition,
// as it seems to be lost otherwise in this context.
// Ideally, common.h should suffice, but this acts as a workaround.
#define NUM_FIELDERS 9

// Enum for different gameplay modes
typedef enum {
    GAMEPLAY_MODE_PLAYING,
    GAMEPLAY_MODE_CUSTOM_FIELDING
} GameplayMode;

// Structure to represent a single ball in an over
typedef struct Ball {
    int runs;
    int is_wicket;
    int is_extra; // 0 = no, 1 = wide, 2 = noball
    struct Ball *next;
} Ball;

// Enum to describe the outcome of a ball
typedef enum {
    OUTCOME_DOT,
    OUTCOME_RUNS,
    OUTCOME_WICKET,
    OUTCOME_EXTRA_WIDE,
    OUTCOME_EXTRA_NOBALL,
    OUTCOME_EXTRA_BYE,
    OUTCOME_EXTRA_LEGBYE
} BallOutcomeType;

// Struct to hold the detailed result of one ball's simulation
typedef struct {
    BallOutcomeType type;
    int runs; // Runs scored on this ball
    float shot_x;
    float shot_y;
    int fielder_index;
    char dismissal_method[32]; // e.g., "Bowled", "Caught", "LBW"
    Player *fielder;           // Pointer to the fielder if applicable
} BallOutcome;

typedef enum {
    POWERPLAY_1,
    POWERPLAY_2,
    POWERPLAY_3
} Powerplay;

// Structure to hold the entire state of a single innings in progress
typedef struct {
    // Pointers to teams - will need special handling for saving/loading
    // For now, assume these are external and will be re-linked on load
    char batting_team_tag[MAX_TEAM_TAG_LEN]; // Store tag to re-link
    char bowling_team_tag[MAX_TEAM_TAG_LEN]; // Store tag to re-link
    Team *batting_team;
    Team *bowling_team;

    int total_runs;
    int wickets;
    int overs_completed;
    int balls_bowled_in_over;

    Powerplay current_powerplay;

    int striker_idx;
    int non_striker_idx;
    int bowler_idx;
    FieldingSetup fielding_setup;

    GameplayMode gameplay_mode; // Current gameplay mode
    Vector2 custom_field_setup[NUM_FIELDERS]; // Storage for custom fielder positions

        int max_overs;

        int target; // 0 if first innings

        float rain_percentage; // Probability of rain per over (0.0 to 1.0)

        char match_log_filename[256]; // Path to the match log file

        FILE *log_file;               // File pointer for ball-by-ball logging

        // Fields to uniquely identify the match for saving/loading GameState
        char match_id[64];
        MatchFormat format;
        int inning_num; // 1 for 1st innings, 2 for 2nd innings

        GameplayPhase phase; // Current phase of the gameplay
    } GameState;

// The new function to simulate a single ball
BallOutcome simulate_one_ball(GameState *state, int inning_num);

// Structure to represent a match
typedef struct {
    Team *teamA;
    Team *teamB;
    MatchFormat format;
    int overs;
    int runsA;
    int wicketsA;
    int runsB;
    int wicketsB;
    char summary[512];
} Match;

int simulate_match(Team *teamA, Team *teamB, MatchFormat format, int autoplay, int rain_possible, const char *umpires, const char* ground, char *out_summary, size_t summary_sz);

// Helper to get the next bowler (now accessible globally)
Player* get_bowler(Team *team, int current_bowler_idx);

// Logging functions
void start_match_log(GameState *state, const char *match_id);
void log_ball_data(GameState *state, int inning_num, int over, int ball_in_over,
                   Player *striker, Player *bowler, int runs_scored, BallOutcomeType outcome_type,
                   const char *dismissal_method, Player *fielder);
void end_match_log(GameState *state);

// Helper to create saves directory
void create_saves_directory(void);

// New: Save and Load GameState functions
bool save_game_state(const GameState *state, const char *filename);
bool load_game_state(GameState *state, const char *filename);

#endif // MATCH_H
