#ifndef MATCH_H
#define MATCH_H

#include "common.h"
#include "teams.h"



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
} BallOutcome;

typedef enum {
    POWERPLAY_1,
    POWERPLAY_2,
    POWERPLAY_3
} Powerplay;

// Structure to hold the entire state of a single innings in progress
typedef struct {
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

} GameState;

// The new function to simulate a single ball
BallOutcome simulate_one_ball(GameState *state);

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

int simulate_match(Team *teamA, Team *teamB, MatchFormat format, int autoplay, int rain_possible, const char *umpire, char *out_summary, size_t summary_sz);

#endif // MATCH_H
