#ifndef TOURNAMENT_H
#define TOURNAMENT_H

#include "common.h"
#include "teams.h"
#include "match.h"

#define MAX_TEAMS_IN_TOURNAMENT 32
#define MAX_MATCHES_IN_TOURNAMENT 64

// Entry for the points table
typedef struct {
    Team *team;
    int played;
    int wins;
    int losses;
    int draws;
    int points;
} PointsTableEntry;

// Structure to represent a tournament
typedef struct {
    char name[128];
    Team teams[MAX_TEAMS_IN_TOURNAMENT];
    int num_teams;
    Match matches[MAX_MATCHES_IN_TOURNAMENT];
    int num_matches;
    MatchFormat format;
    int num_groups;
    PointsTableEntry points_table[MAX_TEAMS_IN_TOURNAMENT];
} Tournament;

void tournament_menu(const char *user_email, const char *user_name);

#endif // TOURNAMENT_H
