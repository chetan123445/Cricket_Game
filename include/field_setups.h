#ifndef FIELD_SETUPS_H
#define FIELD_SETUPS_H

#include "raylib.h"
#include "common.h" // Use common.h instead of match.h
#include "teams.h"    // Required for Player struct definition

#define NUM_FIELD_SETUPS 5
// NUM_FIELDERS is now defined in common.h

extern const Vector2 setups[NUM_FIELD_SETUPS][NUM_FIELDERS];

Vector2* get_field_setup(FieldingSetup setup, Player *striker);

#endif // FIELD_SETUPS_H
