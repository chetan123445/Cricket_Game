#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "match.h"
#include "ui.h"
#include "teams.h"

// Helper to get the next bowler
static Player* get_bowler(Team *team, int current_bowler_idx) {
    // Simple rotation of bowlers
    int next_bowler_idx = (current_bowler_idx + 1) % team->num_players;
    for (int i = 0; i < team->num_players; i++) {
        Player *p = &team->players[next_bowler_idx];
        if (p->type == PLAYER_TYPE_BOWLER || p->type == PLAYER_TYPE_ALLROUNDER) {
            return p;
        }
        next_bowler_idx = (next_bowler_idx + 1) % team->num_players;
    }
    // Fallback to any player if no dedicated bowlers
    return &team->players[next_bowler_idx];
}

BallOutcome simulate_one_ball(GameState *state) {
    BallOutcome outcome = { .type = OUTCOME_DOT, .runs = 0 };

    Player *striker = &state->batting_team->players[state->striker_idx];
    Player *bowler = &state->bowling_team->players[state->bowler_idx];

    int bat_skill = striker->batting_skill;
    int bowl_skill = bowler->bowling_skill;

    // --- Core Simulation Logic (from original autoplay) ---
    int run_chance = 50 + (bat_skill - bowl_skill) / 2;
    if (run_chance < 10) run_chance = 10;
    if (run_chance > 90) run_chance = 90;

    int r = rand() % 100;

    // New: Chance for a "no contact" dot ball
    int no_contact_chance = 20 + (bowl_skill - bat_skill) / 4; // Base 20%, adjusted by skill difference
    if (no_contact_chance < 5) no_contact_chance = 5;   // Minimum 5% chance
    if (no_contact_chance > 50) no_contact_chance = 50; // Maximum 50% chance

    if (r < no_contact_chance) {
        // Explicit "no contact" dot ball
        outcome.type = OUTCOME_DOT;
        outcome.runs = 0;
    } else {
        // Original logic: It's either a scoring shot or a chance for a wicket
        r = rand() % 100; // Re-roll for the next set of probabilities
        if (r < run_chance) {
            // It's a scoring shot
            outcome.type = OUTCOME_RUNS;
            // The actual number of runs is now decided by the player in the GUI.
            // We can still simulate a "potential" run value to guide where the ball goes.
            // For now, we just signal that runs are possible.
            outcome.runs = 1; // Signifies a hit, not the actual run count.
            
        } else {
            // It's a chance for a wicket
        int out_chance = 50 + (bowl_skill - bat_skill) / 4;
        if (out_chance < 10) out_chance = 10;
        if (out_chance > 90) out_chance = 90;

        if ((rand() % 100) < out_chance) {
            outcome.type = OUTCOME_WICKET;
            outcome.runs = 0;
            state->wickets++;
            bowler->total_wickets++;
            state->striker_idx = (state->striker_idx > state->non_striker_idx) ? state->striker_idx + 1 : state->non_striker_idx + 1;

        } else {
            // It's a dot ball
            outcome.type = OUTCOME_DOT;
            outcome.runs = 0;
        }
    }

    // --- Update Game State ---
    state->balls_bowled_in_over++;
    
    // Strike rotation will now be handled by the GUI after runs are taken.
    // if (outcome.runs % 2 != 0) {
    //     int temp = state->striker_idx;
    //     state->striker_idx = state->non_striker_idx;
    //     state->non_striker_idx = temp;
    // }

    // Check for end of over
    if (state->balls_bowled_in_over >= 6) {
        state->overs_completed++;
        state->balls_bowled_in_over = 0;
        // Rotate strike at end of over
        int temp = state->striker_idx;
        state->striker_idx = state->non_striker_idx;
        state->non_striker_idx = temp;

        // --- DYNAMIC RAIN INTERRUPTION LOGIC ---
        // Check for rain at the end of each over.
        if (state->rain_percentage > 0 && (rand() % 100) < (state->rain_percentage * 100)) {
            int overs_lost = 1 + (rand() % 5); // Lose 1 to 5 overs
            if (state->max_overs - overs_lost > state->overs_completed) {
                state->max_overs -= overs_lost;
                printf("\n*** RAIN INTERRUPTION! Match reduced to %d overs. ***\n", state->max_overs);
            }
        }
        // --- END RAIN LOGIC ---

        // Select a new bowler
        int current_bowler_p_idx = -1;
        for(int i=0; i<state->bowling_team->num_players; ++i) {
            if(&state->bowling_team->players[i] == bowler) {
                current_bowler_p_idx = i;
                break;
            }
        }
        Player *new_bowler = get_bowler(state->bowling_team, current_bowler_p_idx);
        for(int i=0; i<state->bowling_team->num_players; ++i) {
             if(&state->bowling_team->players[i] == new_bowler) {
                state->bowler_idx = i;
                break;
            }
        }
    }

    return outcome;
}


static int simulate_innings(Team *batting_team, Team *bowling_team, int max_overs, int autoplay, int *out_runs, int target)
{
    GameState state;
    state.batting_team = batting_team;
    state.bowling_team = bowling_team;
    state.total_runs = 0;
    state.wickets = 0;
    state.overs_completed = 0;
    state.balls_bowled_in_over = 0;
    state.striker_idx = 0;
    state.non_striker_idx = 1;
    state.bowler_idx = -1; // Will be set before first ball
    state.max_overs = max_overs;
    state.target = target;

    // Initial bowler selection
    Player* initial_bowler = get_bowler(bowling_team, -1);
     for(int i=0; i<bowling_team->num_players; ++i) {
        if(&bowling_team->players[i] == initial_bowler) {
            state.bowler_idx = i;
            break;
        }
    }

    // --- Main Innings Loop ---
    while (state.overs_completed < state.max_overs && state.wickets < 10) {
        if (state.overs_completed < 10) {
            state.current_powerplay = POWERPLAY_1;
        } else if (state.overs_completed < 40) {
            state.current_powerplay = POWERPLAY_2;
        } else {
            state.current_powerplay = POWERPLAY_3;
        }

        if (target > 0 && state.total_runs > target) {
             break; // Target chased
        }
        
        Player *striker = &state.batting_team->players[state.striker_idx];
        Player *bowler = &state.bowling_team->players[state.bowler_idx];

        if (state.balls_bowled_in_over == 0) {
            printf("\nOver %d, %s to bowl. Score: %d/%d\n", state.overs_completed + 1, bowler->name, state.total_runs, state.wickets);
        }

        BallOutcome outcome;
        if(autoplay) {
            outcome = simulate_one_ball(&state);
        } else {
             // Manual mode preserved for now
            printf("Ball %d.%d: 1) Dot, 2) 1, 3) 2, 4) 4, 5) 6, 6) Wkt: ", state.overs_completed, state.balls_bowled_in_over+1);
            int choice;
            if (scanf("%d", &choice) != 1) choice = 1;
            clean_stdin();
            
            outcome.runs = 0;
            switch(choice) {
                case 1: outcome.type = OUTCOME_DOT; break;
                case 2: outcome.type = OUTCOME_RUNS; outcome.runs = 1; break;
                case 3: outcome.type = OUTCOME_RUNS; outcome.runs = 2; break;
                case 4: outcome.type = OUTCOME_RUNS; outcome.runs = 4; break;
                case 5: outcome.type = OUTCOME_RUNS; outcome.runs = 6; break;
                case 6: outcome.type = OUTCOME_WICKET; break;
                default: outcome.type = OUTCOME_DOT; break;
            }

            // Manually update state based on choice
            state.balls_bowled_in_over++;
            if (outcome.type == OUTCOME_RUNS) {
                state.total_runs += outcome.runs;
                striker->total_runs += outcome.runs;
                if(outcome.runs % 2 != 0) { int temp = state.striker_idx; state.striker_idx = state.non_striker_idx; state.non_striker_idx = temp; }
            } else if (outcome.type == OUTCOME_WICKET) {
                state.wickets++;
                bowler->total_wickets++;
                state.striker_idx = (state.striker_idx > state.non_striker_idx) ? state.striker_idx + 1 : state.non_striker_idx + 1;
            }
            if (state.balls_bowled_in_over >= 6) {
                state.overs_completed++;
                state.balls_bowled_in_over = 0;
                int temp = state.striker_idx; state.striker_idx = state.non_striker_idx; state.non_striker_idx = temp;
                Player* new_bowler = get_bowler(bowling_team, state.bowler_idx);
                 for(int i=0; i<bowling_team->num_players; ++i) {
                    if(&bowling_team->players[i] == new_bowler) { state.bowler_idx = i; break; }
                }
            }
        }
        
        // --- Print Outcome (replaces old summary logic) ---
        printf(" -> %s on strike. ", striker->name);
        switch(outcome.type) {
            case OUTCOME_DOT: printf("Dot ball.\n"); break;
            case OUTCOME_RUNS: printf("%d runs.\n", outcome.runs); break;
            case OUTCOME_WICKET: printf("WICKET! %s is out!\n", striker->name); break;
            default: printf("...\n"); break;
        }

        if (!autoplay && state.balls_bowled_in_over == 0) {
             printf("End of over. Switch to autoplay? (1 for yes, 0 for no): ");
            int ap_choice;
            if(scanf("%d", &ap_choice) == 1 && ap_choice == 1) {
                autoplay = 1;
            }
            clean_stdin();
        }
    }
    *out_runs = state.total_runs;
    return state.wickets;
}

#include "toss.h"

int simulate_match(Team *teamA, Team *teamB, MatchFormat format, int autoplay, int rain_possible, const char *umpire, char *out_summary, size_t summary_sz)
{
    Team *batting_first = teamA;
    Team *fielding_first = teamB;


    int oversA = 0, oversB = 0;
    if (format == FORMAT_T20) { oversA = oversB = 20; }
    else if (format == FORMAT_ODI) { oversA = oversB = 50; }
    else { oversA = oversB = 90; }
    (void)rain_possible; // This is now handled inside simulate_innings via GameState

    int runsA = 0, runsB = 0;
    printf("\n--- First Innings: %s batting ---\n", batting_first->name);
    int wkA = simulate_innings(batting_first, fielding_first, oversA, autoplay, &runsA, 0);
    
    printf("\n\n--- Innings Break ---\n");
    for (int i = 10; i > 0; i--) {
        printf("\rSecond innings starts in %2d seconds...", i);
        fflush(stdout);
        #if defined(_WIN32)
        Sleep(1000);
        #else
        sleep(1);
        #endif
    }
    printf("\n\n");

    printf("\n--- Second Innings: %s batting ---\n", fielding_first->name);
    int targetB = runsA + 1;
    int wkB = simulate_innings(fielding_first, batting_first, oversB, autoplay, &runsB, targetB);


    // Note: DLS calculation is complex. For now, we'll just compare raw scores.
    // A proper DLS implementation would adjust the target after the rain interruption in the first innings.
    if (runsA == runsB) snprintf(out_summary, summary_sz, "Match Drawn: %s %d - %s %d. Umpire: %s", batting_first->name, runsA, fielding_first->name, runsB, umpire);
    else if (runsA > runsB) snprintf(out_summary, summary_sz, "%s beat %s. Score %d/%d vs %d/%d. Umpire: %s", batting_first->name, fielding_first->name, runsA, wkA, runsB, wkB, umpire);
    else snprintf(out_summary, summary_sz, "%s beat %s. Score %d/%d vs %d/%d. Umpire: %s", fielding_first->name, batting_first->name, runsB, wkB, runsA, wkA, umpire);

    return 1;
}
