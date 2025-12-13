#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include "raylib.h" // For Vector2 and other Raylib types

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

// Enum for the different phases of gameplay animation
typedef enum {
    PHASE_IDLE,
    PHASE_WELCOME, // Initial phase after loading a match
    PHASE_BOWLER_RUNUP,
    PHASE_BALL_TRAVEL,
    PHASE_BATSMAN_SWING,
    PHASE_BALL_IN_FIELD,
    PHASE_PLAY_ENDING,
    PHASE_BOUNDARY_ANIMATION,
    PHASE_BATSMAN_RUNNING,
    PHASE_INNINGS_OVER,
    PHASE_INNINGS_BREAK,
    PHASE_MATCH_OVER
} GameplayPhase;


#endif // COMMON_H
