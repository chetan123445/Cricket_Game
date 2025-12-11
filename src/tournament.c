#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "tournament.h"
#include "match.h"
#include "history.h"
#include "teams.h"
#include "ui.h"

static void run_tournament(MatchFormat format, const char* user_email);

void tournament_menu(const char *user_email, const char *user_name)
{
    (void)user_name; // Mark as unused to prevent warnings
    int running = 1;
    while (running)
    {
        const char *options[] = {"T20 World Cup", "ODI World Cup", "WTC (Test)", "Champions Trophy", "Asia Cup", "Custom Match (pick teams & formats)", "Back"};
        print_menu_box("Tournaments", options, 7);
        printf("Choose: ");
        int c;
        if (scanf("%d", &c) != 1) { scanf("%*s"); clean_stdin(); continue; }
        clean_stdin();
        if (c == 6) // Custom match
        {
            int num_teams = 0;
            Team *teams = load_teams(&num_teams);

            if (num_teams < 2) {
                printf("You need at least 2 teams to play a match. Please create teams from the main menu.\n");
                free(teams);
                continue;
            }

            printf("Available teams:\n");
            for (int i = 0; i < num_teams; i++) {
                printf("%d) %s\n", i + 1, teams[i].name);
            }
            
            int teamA_idx = -1, teamB_idx = -1;
            printf("Choose Team A: ");
            if (scanf("%d", &teamA_idx) != 1) { scanf("%*s"); free(teams); clean_stdin(); continue; }
            clean_stdin();
            printf("Choose Team B: ");
            if (scanf("%d", &teamB_idx) != 1) { scanf("%*s"); free(teams); clean_stdin(); continue; }
            clean_stdin();

            if (teamA_idx < 1 || teamA_idx > num_teams || teamB_idx < 1 || teamB_idx > num_teams || teamA_idx == teamB_idx) {
                printf("Invalid team selection.\n");
                free(teams);
                continue;
            }

            Team *teamA = &teams[teamA_idx - 1];
            Team *teamB = &teams[teamB_idx - 1];
            
            char umpire[64] = "Default Umpire";
            int autoplay = 1, rain = 1;
            MatchFormat fmt = FORMAT_T20;
            
            printf("Choose format: 1)T20 2)ODI 3)Test: "); 
            int ff; 
            if (scanf("%d", &ff) != 1) ff = 1;
            clean_stdin();
            if (ff == 2) fmt = FORMAT_ODI; 
            else if (ff == 3) fmt = FORMAT_TEST; 
            else fmt = FORMAT_T20;
            
            printf("Umpire name (or Enter for default): "); 
            if (!fgets(umpire, sizeof(umpire), stdin) || umpire[0] == '\n') {
                strcpy(umpire, "Default Umpire");
            }
            umpire[strcspn(umpire, "\r\n")] = 0;
            
            printf("Enable autoplay? (1 for yes, 0 for no): ");
            if (scanf("%d", &autoplay) != 1) autoplay = 1;
            clean_stdin();

            char summary[512] = {0};
            simulate_match(teamA, teamB, fmt, autoplay, rain, umpire, summary, sizeof(summary));
            printf("\nMatch result:\n%s\n", summary);
            append_user_history(user_email, summary);
            free(teams);
        }
        else if (c >= 1 && c <= 5) {
            MatchFormat fmt = FORMAT_T20;
            if (c == 2 || c == 4 || c==5) fmt = FORMAT_ODI;
            else if (c == 3) fmt = FORMAT_TEST;
            run_tournament(fmt, user_email);
        }
        else if (c == 7) running = 0;
        else printf("Invalid.\n");
    }
}

static void create_tournament(Tournament *t, Team *all_teams, int num_all_teams) {
    printf("\n--- Create Tournament ---\n");
    printf("Available teams:\n");
    for (int i = 0; i < num_all_teams; i++) {
        printf("%d) %s\n", i + 1, all_teams[i].name);
    }

    printf("Enter number of teams for the tournament: ");
    if (scanf("%d", &t->num_teams) != 1 || t->num_teams < 2 || t->num_teams > num_all_teams) {
        printf("Invalid number of teams.\n");
        t->num_teams = 0;
        clean_stdin();
        return;
    }
    clean_stdin();

    printf("Choose teams by number (e.g., 1 2 3 4): ");
    for (int i = 0; i < t->num_teams; i++) {
        int team_idx;
        if (scanf("%d", &team_idx) != 1 || team_idx < 1 || team_idx > num_all_teams) {
            printf("Invalid selection.\n");
            i--; // retry
            clean_stdin();
            continue;
        }
        clean_stdin();
        t->teams[i] = all_teams[team_idx - 1];
    }

    // Initialize points table
    for (int i = 0; i < t->num_teams; i++) {
        t->points_table[i].team = &t->teams[i];
        t->points_table[i].played = 0;
        t->points_table[i].wins = 0;
        t->points_table[i].losses = 0;
        t->points_table[i].draws = 0;
        t->points_table[i].points = 0;
    }
    t->num_matches = 0;
}

static void generate_fixtures(Tournament *t) {
    if (t->num_teams < 2) return;

    t->num_matches = 0;
    for (int i = 0; i < t->num_teams; i++) {
        for (int j = i + 1; j < t->num_teams; j++) {
            if (t->num_matches < MAX_MATCHES_IN_TOURNAMENT) {
                Match *m = &t->matches[t->num_matches++];
                m->teamA = &t->teams[i];
                m->teamB = &t->teams[j];
                m->format = t->format;
            }
        }
    }
}

static void display_points_table(const Tournament *t) {
    printf("\n--- Points Table: %s ---\n", t->name);
    printf("%-20s | P | W | L | D | Pts\n", "Team");
    printf("------------------------------------------------\n");
    for (int i = 0; i < t->num_teams; i++) {
        const PointsTableEntry *entry = &t->points_table[i];
        printf("%-20s | %d | %d | %d | %d | %d\n",
               entry->team->name, entry->played, entry->wins, entry->losses, entry->draws, entry->points);
    }
    printf("------------------------------------------------\n");
}

static void run_tournament(MatchFormat format, const char* user_email) {
    int num_all_teams = 0;
    Team *all_teams = load_teams(&num_all_teams);
    if (num_all_teams < 2) {
        printf("Not enough teams to run a tournament. Please create more teams.\n");
        free(all_teams);
        return;
    }

    Tournament t = {0};
    strncpy(t.name, "My Tournament", sizeof(t.name) - 1);
    t.format = format;

    create_tournament(&t, all_teams, num_all_teams);
    
    if (t.num_teams < 2) {
        free(all_teams);
        return;
    }

    generate_fixtures(&t);
    printf("Tournament created with %d teams and %d matches.\n", t.num_teams, t.num_matches);

    int autoplay = 1;
    printf("Enable autoplay for all matches? (1 for yes, 0 for no): ");
    if (scanf("%d", &autoplay) != 1) autoplay = 1;
    clean_stdin();

    for (int i = 0; i < t.num_matches; i++) {
        Match *m = &t.matches[i];
        printf("\nSimulating match: %s vs %s\n", m->teamA->name, m->teamB->name);
        char summary[512] = {0};
        simulate_match(m->teamA, m->teamB, m->format, autoplay, 1, "Default Umpire", summary, sizeof(summary));
        printf("%s\n", summary);
        append_user_history(user_email, summary);

        // Update points table
        PointsTableEntry *entryA = NULL, *entryB = NULL;
        for (int j = 0; j < t.num_teams; j++) {
            if (t.points_table[j].team == m->teamA) entryA = &t.points_table[j];
            if (t.points_table[j].team == m->teamB) entryB = &t.points_table[j];
        }

        if (entryA && entryB) {
            entryA->played++;
            entryB->played++;
            if (strstr(summary, m->teamA->name) == summary) { // Team A won
                entryA->wins++;
                entryA->points += 2;
                entryB->losses++;
            } else if (strstr(summary, m->teamB->name) == summary) { // Team B won
                entryB->wins++;
                entryB->points += 2;
                entryA->losses++;
            } else { // Draw
                entryA->draws++;
                entryB->draws++;
                entryA->points += 1;
                entryB->points += 1;
            }
        }
    }

    display_points_table(&t);

    free(all_teams);
}
