#include "field_setups.h"

const Vector2 setups[NUM_FIELD_SETUPS][NUM_FIELDERS] = {
    // PP_AGGRESSIVE (Powerplay Aggressive)
    {
    {  0.30, -0.40 },   // Slip 1 (Outside)
    {  0.20, -0.45 },   // Slip 2 (Outside)
    { -0.15,  0.10 },   // Short Mid-wicket (Inside)
    {  0.15,  0.10 },   // Short Cover (Inside)
    { -0.50,  0.40 },   // Square Leg (Outside)
    {  0.50,  0.40 },   // Point (Outside)
    { -0.80,  0.60 },   // Fine Leg (Outside)
    {  0.80,  0.60 },   // Third Man (Outside)
    {  0.00,  0.55 }    // Mid-off (Outside)
    },
    // PP_DEFENSIVE (Powerplay Defensive)
    {
    {  0.25, -0.45 },   // Fly Slip (Outside)
    {  0.35, -0.35 },   // Gully (Outside)
    { -0.20,  0.15 },   // Mid-wicket (Inside)
    {  0.20,  0.15 },   // Cover (Inside)
    { -0.40,  0.40 },   // Square Leg (Outside)
    {  0.40,  0.40 },   // Point (Outside)
    { -0.75,  0.65 },   // Deep Square Leg (Outside)
    {  0.75,  0.65 },   // Deep Point (Outside)
    {  0.00,  0.70 }    // Long-off (Outside)
    },
    // MO_NORMAL (Middle Overs - Normal)
    {
    { -0.35,  0.15 },   // Mid-wicket (slightly deeper)
    {  0.35,  0.15 },   // Cover (slightly deeper)
    { -0.20,  0.30 },   // Square Leg (inside circle)
    {  0.20,  0.30 },   // Point (inside circle)
    { -0.75,  0.65 },   // Deep Square Leg (boundary)
    {  0.75,  0.65 },   // Deep Cover (boundary)
    {  0.00,  0.70 },   // Long-off (deep, boundary protection)
    {  0.00, -0.30 },   // Short Fine Leg (attacking)
    {  0.00,  0.20 }    // Mid-on (closer in)
    },
    // MO_DEFENSIVE (Middle Overs - Defensive)
    {
    { -0.45,  0.25 },   // Mid-wicket (deeper)
    {  0.45,  0.25 },   // Cover (deeper)
    { -0.20,  0.40 },   // Square Leg (further back)
    {  0.20,  0.40 },   // Point (further back)
    { -0.80,  0.70 },   // Deep Square Leg (boundary)
    {  0.80,  0.70 },   // Deep Point (boundary)
    {  0.00,  0.80 },   // Long-off (boundary)
    {  0.00,  0.60 },   // Long-on (boundary)
    {  0.00, -0.10 }    // Short Fine Leg (still somewhat attacking)
    },
    // DO_DEFENSIVE (Death Overs Defensive)
    {
    { -0.90,  0.70 },   // Deep Square Leg (boundary)
    { -0.70,  0.60 },   // Deep Mid-wicket (boundary)
    {  0.70,  0.60 },   // Deep Extra Cover (boundary)
    {  0.90,  0.70 },   // Deep Point (boundary)
    {  0.00,  0.85 },   // Long-off (very deep boundary)
    {  0.00,  0.75 },   // Long-on (very deep boundary)
    { -0.15,  0.40 },   // Short Mid-wicket (inside 30, saving 1s)
    {  0.15,  0.40 },   // Short Cover (inside 30, saving 1s)
    {  0.00, -0.05 }    // Fine Leg (inside 30, for Yorkers)
    },
};

Vector2* get_field_setup(FieldingSetup setup, Player *striker) {
    static Vector2 mirrored_setup[NUM_FIELDERS];
    if (striker->batting_style == BATTING_STYLE_LHB) {
        for (int i = 0; i < NUM_FIELDERS; i++) {
            mirrored_setup[i].x = -setups[setup][i].x;
            mirrored_setup[i].y = setups[setup][i].y;
        }
        return mirrored_setup;
    }
    return (Vector2*)setups[setup];
}
