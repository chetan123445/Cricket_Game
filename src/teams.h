#ifndef TEAMS_H
#define TEAMS_H

#include "common.h"
#include <stdbool.h>

#define MAX_PLAYERS 22
#define MAX_TEAM_NAME_LEN 64
#define MAX_TEAM_TAG_LEN 10
#define MAX_PLAYER_NAME_LEN 64

// New Enums for detailed player attributes
typedef enum {
    BATTING_STYLE_RHB,
    BATTING_STYLE_LHB
} BattingStyle;

typedef enum {
    BOWLING_STYLE_NONE,
    // Pace
    BOWLING_STYLE_RF,   // Right-arm fast
    BOWLING_STYLE_RFM,  // Right-arm fast-medium
    BOWLING_STYLE_RM,   // Right-arm medium
    BOWLING_STYLE_LF,   // Left-arm fast
    BOWLING_STYLE_LFM,  // Left-arm fast-medium
    BOWLING_STYLE_LM,   // Left-arm medium
    // Spin
    BOWLING_STYLE_OB,   // Right-arm off-spin
    BOWLING_STYLE_LB,   // Right-arm leg-spin
    BOWLING_STYLE_SLA,  // Left-arm orthodox spin
    BOWLING_STYLE_LWS   // Left-arm wrist-spin (Chinaman)
} BowlingStyle;

// Structure to represent a player
typedef struct {
    char name[MAX_PLAYER_NAME_LEN];
    PlayerType type;

    // Detailed attributes
    BattingStyle batting_style;
    BowlingStyle bowling_style;
    bool is_wicketkeeper;
    bool is_active;

    // Skills (0-100)
    int batting_skill;
    int bowling_skill;
    int fielding_skill;

    // Career Stats
    int matches_played;
    int total_runs;
    int total_wickets;
    int total_stumpings;
    int total_run_outs;
} Player;

// Structure to represent a team
typedef struct {
    char name[MAX_TEAM_NAME_LEN];
    char tag[10]; // Short tag for the team, e.g., "IND", "AUS"
    Player players[MAX_PLAYERS];
    int num_players;
    int captain_idx;
    int vice_captain_idx;
    bool is_deleted;
    bool is_hidden;
} Team;

void teams_menu(const char *user_email);
void umpires_menu(const char *user_email);

// New functions for team management
Team* load_teams(int *num_teams);
void save_teams(const Team *teams, int num_teams);
void show_player_stats(const Team *team);
Player* load_global_players(int *num_players);

void initialize_dummy_teams(Team *teamA, Team *teamB);

// New function to retrieve a team by its tag
Team* get_team_by_tag(const char *tag);

#endif // TEAMS_H
