#ifndef MATCH_H
#define MATCH_H

#include "common.h"
#include "teams.h"

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
} BallOutcome;

// Structure to hold the entire state of a single innings in progress
typedef struct {
    Team *batting_team;
    Team *bowling_team;

    int total_runs;
    int wickets;
    int overs_completed;
    int balls_bowled_in_over;

    int striker_idx;
    int non_striker_idx;
    int bowler_idx;

    int max_overs;
    int target; // 0 if first innings

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
