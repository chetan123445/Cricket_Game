#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h> // For _mkdir on Windows
#include <errno.h>  // For checking errno
#include "match.h"
#include "ui.h"
#include "teams.h"

// Helper to get the next bowler
Player* get_bowler(Team *team, int current_bowler_idx) {
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

// Helper function to generate a unique filename for the match log
static void generate_match_log_filename(char *buffer, size_t buffer_size, const char *match_id) {
    time_t rawtime;
    struct tm *info;
    char time_buffer[80];

    time(&rawtime);
    info = localtime(&rawtime);

    strftime(time_buffer, 80, "%Y%m%d_%H%M%S", info);
    snprintf(buffer, buffer_size, "Data/match_%s_%s.log", match_id, time_buffer);
}

void start_match_log(GameState *state, const char *match_id) {
    generate_match_log_filename(state->match_log_filename, sizeof(state->match_log_filename), match_id);
    state->log_file = fopen(state->match_log_filename, "w");
    if (state->log_file) {
        fprintf(state->log_file, "Inning,Over,Ball,Bowler,Batsman,Runs,Outcome,DismissalMethod,Fielder\n");
    } else {
        fprintf(stderr, "Error: Could not open match log file %s\n", state->match_log_filename);
    }
}

void log_ball_data(GameState *state, int inning_num, int over, int ball_in_over,
                   Player *striker, Player *bowler, int runs_scored, BallOutcomeType outcome_type,
                   const char *dismissal_method, Player *fielder) {
    if (!state->log_file) return;

    fprintf(state->log_file, "%d,%d,%d,%s,%s,%d,", inning_num, over, ball_in_over,
            bowler->name, striker->name, runs_scored);

    switch (outcome_type) {
        case OUTCOME_DOT: fprintf(state->log_file, "DOT,"); break;
        case OUTCOME_RUNS: fprintf(state->log_file, "RUNS,"); break;
        case OUTCOME_WICKET: fprintf(state->log_file, "WICKET,"); break;
        case OUTCOME_EXTRA_WIDE: fprintf(state->log_file, "WIDE,"); break;
        case OUTCOME_EXTRA_NOBALL: fprintf(state->log_file, "NOBALL,"); break;
        case OUTCOME_EXTRA_BYE: fprintf(state->log_file, "BYE,"); break;
        case OUTCOME_EXTRA_LEGBYE: fprintf(state->log_file, "LEGBYE,"); break;
    }

    if (dismissal_method) {
        fprintf(state->log_file, "%s,", dismissal_method);
    } else {
        fprintf(state->log_file, ","); // Empty for no dismissal
    }

    if (fielder) {
        fprintf(state->log_file, "%s\n", fielder->name);
    } else {
        fprintf(state->log_file, "\n"); // Empty for no fielder
    }
    fflush(state->log_file); // Ensure data is written immediately
}

void end_match_log(GameState *state) {
    if (state->log_file) {
        fclose(state->log_file);
        state->log_file = NULL;
    }
}

BallOutcome simulate_one_ball(GameState *state, int inning_num) {
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
        if (out_chance > 90) run_chance = 90;

        if ((rand() % 100) < out_chance) {
            outcome.type = OUTCOME_WICKET;
            outcome.runs = 0;
            state->wickets++;
            bowler->total_wickets++;
            state->striker_idx = (state->striker_idx > state->non_striker_idx) ? state->striker_idx + 1 : state->non_striker_idx + 1;

            // Determine dismissal method (50/50 for now)
            if (rand() % 2 == 0) {
                strcpy(outcome.dismissal_method, "Bowled");
                outcome.fielder = NULL; // No fielder for bowled
            } else {
                strcpy(outcome.dismissal_method, "Caught (Simulated)");
                // For simplicity, for caught, we won't assign a specific fielder in CLI mode for now, just mark it as caught
                outcome.fielder = NULL;
            }
        } else {
            // It's a dot ball
            outcome.type = OUTCOME_DOT;
            outcome.runs = 0;
            strcpy(outcome.dismissal_method, ""); // No dismissal
            outcome.fielder = NULL;
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
    // Log ball data here, after all state updates for the ball
    log_ball_data(state, inning_num, state->overs_completed, state->balls_bowled_in_over,
                  striker, bowler, outcome.runs, outcome.type,
                  outcome.dismissal_method, outcome.fielder);

    return outcome;
}

// Helper function to create the Data/saves directory if it doesn't exist
void create_saves_directory() {
    #if defined(_WIN32)
        _mkdir("Data/saves");
    #else
        mkdir("Data/saves", 0777); // Permissions 0777 for broader compatibility
    #endif
}

static int simulate_innings(GameState *state_ptr, Team *batting_team_param, Team *bowling_team_param, int max_overs_param, int autoplay_param, int *out_runs_param, int target_param, int inning_num)
{
    // Use the passed state_ptr
    GameState *state = state_ptr; // Alias for convenience

    // Re-link teams from tags if this is a loaded game state
    // This is crucial if simulate_innings is called as part of a loaded game
    if (state->batting_team == NULL && strlen(state->batting_team_tag) > 0) {
        state->batting_team = get_team_by_tag(state->batting_team_tag);
    }
    if (state->bowling_team == NULL && strlen(state->bowling_team_tag) > 0) {
        state->bowling_team = get_team_by_tag(state->bowling_team_tag);
    }
    if (!state->batting_team || !state->bowling_team) {
        fprintf(stderr, "Error: Teams could not be re-linked in simulate_innings.\n");
        return -1; // Indicate error
    }

    // Initialize these only if it's a fresh start for innings, not a loaded one.
    // This assumes the initial call to simulate_innings for a new match will have these values as 0.
    // If a game is loaded, these will have non-zero values.
    if (state->total_runs == 0 && state->wickets == 0 && state->overs_completed == 0 && state->balls_bowled_in_over == 0) {
        state->batting_team = batting_team_param; // Reassign for fresh innings
        state->bowling_team = bowling_team_param; // Reassign for fresh innings
        state->total_runs = 0;
        state->wickets = 0;
        state->overs_completed = 0;
        state->balls_bowled_in_over = 0;
        state->striker_idx = 0;
        state->non_striker_idx = 1;
        state->bowler_idx = -1; // Will be set before first ball
    }
    
    state->max_overs = max_overs_param;
    state->target = target_param;
    // state->rain_percentage already set in simulate_match

    // Initial bowler selection (only if starting fresh or bowler_idx is -1)
    if (state->bowler_idx == -1) {
        Player* initial_bowler = get_bowler(state->bowling_team, -1);
        for(int i=0; i<state->bowling_team->num_players; ++i) {
            if(&state->bowling_team->players[i] == initial_bowler) {
                state->bowler_idx = i;
                break;
            }
        }
    }

    // --- Main Innings Loop ---
    while (state->overs_completed < state->max_overs && state->wickets < 10) {
        // Powerplay logic is now handled in GUI if applicable, not in CLI simulation
        // if (state->overs_completed < 10) {
        //     state->current_powerplay = POWERPLAY_1;
        // } else if (state->overs_completed < 40) {
        //     state->current_powerplay = POWERPLAY_2;
        // } else {
        //     state->current_powerplay = POWERPLAY_3;
        // }

        if (state->target > 0 && state->total_runs > state->target) {
             break; // Target chased
        }
        
        Player *striker = &state->batting_team->players[state->striker_idx];
        Player *bowler = &state->bowling_team->players[state->bowler_idx];

        if (state->balls_bowled_in_over == 0) {
            printf("\nOver %d, %s to bowl. Score: %d/%d\n", state->overs_completed + 1, bowler->name, state->total_runs, state->wickets);
        }

        BallOutcome outcome;
        if(autoplay_param) {
            outcome = simulate_one_ball(state, inning_num); // Pass state pointer and inning_num
            // Log ball data immediately after simulation
            log_ball_data(state, inning_num, state->overs_completed, state->balls_bowled_in_over,
                          striker, bowler, outcome.runs, outcome.type,
                          (outcome.type == OUTCOME_WICKET) ? "Simulated Wicket" : NULL, // Simplified dismissal for now
                          NULL);
        } else {
             // Manual mode preserved for now
            printf("Ball %d.%d: 1) Dot, 2) 1, 3) 2, 4) 4, 5) 6, 6) Wkt: ", state->overs_completed, state->balls_bowled_in_over+1);
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
            state->balls_bowled_in_over++;
            if (outcome.type == OUTCOME_RUNS) {
                state->total_runs += outcome.runs;
                striker->total_runs += outcome.runs;
                if(outcome.runs % 2 != 0) { int temp = state->striker_idx; state->striker_idx = state->non_striker_idx; state->non_striker_idx = temp; }
            } else if (outcome.type == OUTCOME_WICKET) {
                state->wickets++;
                bowler->total_wickets++;
                state->striker_idx = (state->striker_idx > state->non_striker_idx) ? state->striker_idx + 1 : state->non_striker_idx + 1;
            }
            if (state->balls_bowled_in_over >= 6) {
                state->overs_completed++;
                state->balls_bowled_in_over = 0;
                int temp = state->striker_idx; state->striker_idx = state->non_striker_idx; state->non_striker_idx = temp;
                Player* new_bowler = get_bowler(state->bowling_team, state->bowler_idx);
                 for(int i=0; i<state->bowling_team->num_players; ++i) {
                    if(&state->bowling_team->players[i] == new_bowler) { state->bowler_idx = i; break; }
                }
            }
            // Log ball data for manual mode
            log_ball_data(state, inning_num, state->overs_completed, state->balls_bowled_in_over,
                          striker, bowler, outcome.runs, outcome.type,
                          (outcome.type == OUTCOME_WICKET) ? "Manual Wicket" : NULL,
                          NULL);
        }
        
        // --- Print Outcome (replaces old summary logic) ---
        printf(" -> %s on strike. ", striker->name);
        switch(outcome.type) {
            case OUTCOME_DOT: printf("Dot ball.\n"); break;
            case OUTCOME_RUNS: printf("%d runs.\n", outcome.runs); break;
            case OUTCOME_WICKET: printf("WICKET! %s is out!\n", striker->name); break;
            default: printf("...\n"); break;
        }

        if (!autoplay_param && state->balls_bowled_in_over == 0) {
             printf("End of over. Switch to autoplay? (1 for yes, 0 for no): ");
            int ap_choice;
            if(scanf("%d", &ap_choice) == 1 && ap_choice == 1) {
                autoplay_param = 1;
            }
            clean_stdin();
        }
    }
    *out_runs_param = state->total_runs;
    return state->wickets;
}

// Function to save the current GameState to a file
bool save_game_state(const GameState *state, const char *filename) {
    FILE *f = fopen(filename, "w");
    if (!f) {
        fprintf(stderr, "Error: Could not open save file %s for writing.\n", filename);
        return false;
    }

    fprintf(f, "batting_team_tag: %s\n", state->batting_team_tag);
    fprintf(f, "bowling_team_tag: %s\n", state->bowling_team_tag);
    fprintf(f, "total_runs: %d\n", state->total_runs);
    fprintf(f, "wickets: %d\n", state->wickets);
    fprintf(f, "overs_completed: %d\n", state->overs_completed);
    fprintf(f, "balls_bowled_in_over: %d\n", state->balls_bowled_in_over);
    fprintf(f, "current_powerplay: %d\n", state->current_powerplay);
    fprintf(f, "striker_idx: %d\n", state->striker_idx);
    fprintf(f, "non_striker_idx: %d\n", state->non_striker_idx);
    fprintf(f, "bowler_idx: %d\n", state->bowler_idx);
    fprintf(f, "max_overs: %d\n", state->max_overs);
    fprintf(f, "target: %d\n", state->target);
    fprintf(f, "rain_percentage: %f\n", state->rain_percentage);
    fprintf(f, "match_id: %s\n", state->match_id);
    fprintf(f, "format: %d\n", state->format);
    fprintf(f, "inning_num: %d\n", state->inning_num);

    fclose(f);
    return true;
}

// Function to load GameState from a file
bool load_game_state(GameState *state, const char *filename) {
    FILE *f = fopen(filename, "rb"); // Use "rb" for binary-safe reading on all platforms
    if (!f) {
        fprintf(stderr, "Error: Could not open save file %s for reading.\n", filename);
        return false;
    }

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        int int_val;
        float float_val;
        char str_val[100];

        if (sscanf(line, "batting_team_tag: %99s", str_val) == 1) {
            strcpy(state->batting_team_tag, str_val);
        } else if (sscanf(line, "bowling_team_tag: %99s", str_val) == 1) {
            strcpy(state->bowling_team_tag, str_val);
        } else if (sscanf(line, "total_runs: %d", &int_val) == 1) {
            state->total_runs = int_val;
        } else if (sscanf(line, "wickets: %d", &int_val) == 1) {
            state->wickets = int_val;
        } else if (sscanf(line, "overs_completed: %d", &int_val) == 1) {
            state->overs_completed = int_val;
        } else if (sscanf(line, "balls_bowled_in_over: %d", &int_val) == 1) {
            state->balls_bowled_in_over = int_val;
        } else if (sscanf(line, "current_powerplay: %d", &int_val) == 1) {
            state->current_powerplay = (Powerplay)int_val;
        } else if (sscanf(line, "striker_idx: %d", &int_val) == 1) {
            state->striker_idx = int_val;
        } else if (sscanf(line, "non_striker_idx: %d", &int_val) == 1) {
            state->non_striker_idx = int_val;
        } else if (sscanf(line, "bowler_idx: %d", &int_val) == 1) {
            state->bowler_idx = int_val;
        } else if (sscanf(line, "max_overs: %d", &int_val) == 1) {
            state->max_overs = int_val;
        } else if (sscanf(line, "target: %d", &int_val) == 1) {
            state->target = int_val;
        } else if (sscanf(line, "rain_percentage: %f", &float_val) == 1) {
            state->rain_percentage = float_val;
        } else if (sscanf(line, "match_id: %99s", str_val) == 1) {
            strcpy(state->match_id, str_val);
        } else if (sscanf(line, "format: %d", &int_val) == 1) {
            state->format = (MatchFormat)int_val;
        } else if (sscanf(line, "inning_num: %d", &int_val) == 1) {
            state->inning_num = int_val;
        }
    }

    fclose(f);

    // Re-link team pointers using the loaded tags
    state->batting_team = get_team_by_tag(state->batting_team_tag);
    state->bowling_team = get_team_by_tag(state->bowling_team_tag);

    if (!state->batting_team || !state->bowling_team) {
        fprintf(stderr, "Error: Could not re-link teams after loading game state from tags '%s' and '%s'.\n", state->batting_team_tag, state->bowling_team_tag);
        return false; // Cannot proceed
    }
    
    // Crucially, set the phase to start the loaded game
    state->phase = PHASE_WELCOME;

    return true;
}

int simulate_match(Team *teamA, Team *teamB, MatchFormat format, int autoplay, int rain_possible, const char *umpires, const char* ground, char *out_summary, size_t summary_sz)
{
    create_saves_directory(); // Ensure the saves directory exists
    // Generate a unique match ID
    char match_id[64];
    time_t t;
    srand((unsigned) time(&t)); // Initialize random seed for match ID
    sprintf(match_id, "%s_VS_%s_%lld", teamA->tag, teamB->tag, time(NULL));

    // Initialize GameState for the entire match
    GameState match_state = { 0 };
    strncpy(match_state.batting_team_tag, teamA->tag, MAX_TEAM_TAG_LEN - 1);
    match_state.batting_team_tag[MAX_TEAM_TAG_LEN - 1] = '\0';
    strncpy(match_state.bowling_team_tag, teamB->tag, MAX_TEAM_TAG_LEN - 1);
    match_state.bowling_team_tag[MAX_TEAM_TAG_LEN - 1] = '\0';
    match_state.batting_team = teamA;
    match_state.bowling_team = teamB;
    match_state.max_overs = 0; // Will be set per innings
    match_state.target = 0;
    match_state.rain_percentage = rain_possible ? 0.1f : 0.0f; // Simplified rain for now
    match_state.log_file = NULL; // Ensure it's NULL initially
    strncpy(match_state.match_id, match_id, sizeof(match_state.match_id) - 1);
    match_state.match_id[sizeof(match_state.match_id) - 1] = '\0';
    match_state.format = format;
    match_state.inning_num = 0; // Will be set before each innings

    start_match_log(&match_state, match_id); // Start logging for the match

    printf("\n--- Let's Play! ---\n");
    printf("%s vs %s\n", teamA->name, teamB->name);
    printf("Ground: %s\n", ground);
    printf("Umpires: %s\n", umpires);

    // Toss
    printf("Time for the toss. Heads (H) or Tails (T)? ");
    char toss_call;
    scanf(" %c", &toss_call);
    clean_stdin();
    int toss_outcome = rand() % 2; // 0 for heads, 1 for tails
    int user_won_toss = (toss_outcome == 0 && (toss_call == 'H' || toss_call == 'h')) || (toss_outcome == 1 && (toss_call == 'T' || toss_call == 't'));
    
    Team *batting_first, *fielding_first;

    if (user_won_toss) {
        printf("You won the toss! Bat (1) or Field (2)? ");
        int choice;
        if (scanf("%d", &choice) != 1) choice = 1;
        clean_stdin();
        if (choice == 1) {
            batting_first = teamA;
            fielding_first = teamB;
        } else {
            batting_first = teamB;
            fielding_first = teamA;
        }
    } else {
        printf("You lost the toss. The opponent will choose.\n");
        int opp_choice = rand() % 2;
        if (opp_choice == 0) {
            printf("Opponent chose to bat.\n");
            batting_first = teamB;
            fielding_first = teamA;
        } else {
            printf("Opponent chose to field.\n");
            batting_first = teamA;
            fielding_first = teamB; // Corrected: was teamA
        }
    }
    int overs_per_innings = 0;
    if (format == FORMAT_T20) { overs_per_innings = 20; }
    else if (format == FORMAT_ODI) { overs_per_innings = 50; }
    else { overs_per_innings = 90; }

    int runsA = 0, runsB = 0;
    int wkA = 0, wkB = 0;

    printf("\n--- First Innings: %s batting ---\n", batting_first->name);
    // Passing match_state (GameState pointer) and inning number (1)
    match_state.inning_num = 1;
    wkA = simulate_innings(&match_state, batting_first, fielding_first, overs_per_innings, autoplay, &runsA, 0, 1);
    
    printf("\n--- Second Innings: %s batting ---\n", fielding_first->name);
    match_state.target = runsA + 1; // Set target for second innings
    // Passing match_state (GameState pointer) and inning number (2)
    match_state.inning_num = 2;
    wkB = simulate_innings(&match_state, fielding_first, batting_first, overs_per_innings, autoplay, &runsB, match_state.target, 2);

    // Note: DLS calculation is complex. For now, we'll just compare raw scores.
    // A proper DLS implementation would adjust the target after the rain interruption in the first innings.
    if (runsA == runsB) snprintf(out_summary, summary_sz, "Match Drawn: %s %d - %s %d. Ground: %s, Umpires: %s", batting_first->name, runsA, fielding_first->name, runsB, ground, umpires);
    else if (runsA > runsB) snprintf(out_summary, summary_sz, "%s beat %s. Score %d/%d vs %d/%d. Ground: %s, Umpires: %s", batting_first->name, fielding_first->name, runsA, wkA, runsB, wkB, ground, umpires);
    else snprintf(out_summary, summary_sz, "%s beat %s. Score %d/%d vs %d/%d. Ground: %s, Umpires: %s", fielding_first->name, batting_first->name, runsB, wkB, runsA, wkA, ground, umpires);

    end_match_log(&match_state); // End logging for the match
    return 1;
}

