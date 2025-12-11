#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>

// Enum for player types
typedef enum {
    PLAYER_TYPE_BATSMAN,
    PLAYER_TYPE_BOWLER,
    PLAYER_TYPE_ALLROUNDER,
    PLAYER_TYPE_WICKETKEEPER
} PlayerType;

// Enum for different fielding presets
typedef enum {
    PP_AGGRESSIVE,
    PP_DEFENSIVE,
    MO_NORMAL,
    MO_DEFENSIVE,
    DO_DEFENSIVE,
    FIELD_SETUP_COUNT // To know the number of setups
} FieldingSetup;

#define NUM_FIELDERS 9 // Number of fielders excluding bowler and wicket-keeper

// Enum for match formats
typedef enum {
    FORMAT_T20,
    FORMAT_ODI,
    FORMAT_TEST
} MatchFormat;

#endif // COMMON_H
