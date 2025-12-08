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

// Enum for match formats
typedef enum {
    FORMAT_T20,
    FORMAT_ODI,
    FORMAT_TEST
} MatchFormat;

#endif // COMMON_H
