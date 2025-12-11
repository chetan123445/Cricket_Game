#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "teams.h"
#include "ui.h"

static const char *TEAMS_FILE = "Data/teams.dat";
static const char *PLAYERS_FILE = "Data/players.dat";
static const char *UMPIRES_FILE = "Data/umpires.dat";
static const char *ALL_PLAYERS_FILE = "Data/all_players.dat";

Team* load_teams(int *num_teams);
void save_teams(const Team *teams, int num_teams);
void show_player_stats(const Team *team);
Player* load_global_players(int *num_players);


Player* load_global_players(int *num_players) {
    FILE *f = fopen(ALL_PLAYERS_FILE, "r");
    if (!f) {
        *num_players = 0;
        return NULL;
    }

    char line[256];
    int count = 0;
    while(fgets(line, sizeof(line), f)) {
        count++;
    }
    rewind(f);

    if (count == 0) {
        *num_players = 0;
        fclose(f);
        return NULL;
    }

    Player *players = (Player*)malloc(count * sizeof(Player));
    if (!players) {
        *num_players = 0;
        fclose(f);
        return NULL;
    }

    int i = 0;
    while(fgets(line, sizeof(line), f)) {
        char name[MAX_PLAYER_NAME_LEN];
        int type, bat, bowl;
        if (sscanf(line, "%[^,],%d,%d,%d", name, &type, &bat, &bowl) == 4) {
            strncpy(players[i].name, name, MAX_PLAYER_NAME_LEN - 1);
            players[i].name[MAX_PLAYER_NAME_LEN-1] = '\0';
            players[i].type = (PlayerType)type;
            players[i].batting_skill = bat;
            players[i].bowling_skill = bowl;
            players[i].total_runs = 0;
            players[i].total_wickets = 0;
            players[i].total_stumpings = 0;
            players[i].total_run_outs = 0;
            players[i].matches_played = 0;
            i++;
        }
    }
    *num_players = i;
    fclose(f);
    return players;
}


Team* load_teams(int *num_teams) {
    FILE *f = fopen(TEAMS_FILE, "r");
    if (!f) {
        *num_teams = 0;
        return NULL;
    }

    // Count teams
    char line[512];
    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        if (strlen(line) > 1) {
            count++;
        }
    }
    rewind(f);

    if (count == 0) {
        *num_teams = 0;
        fclose(f);
        return NULL;
    }

    Team *teams = (Team*)malloc(count * sizeof(Team));
    if (!teams) {
        *num_teams = 0;
        fclose(f);
        return NULL;
    }

    int i = 0;
    while (fgets(line, sizeof(line), f) && i < count) {
        line[strcspn(line, "\r\n")] = 0;
        char *token;

        token = strtok(line, ",");
        if (token == NULL) continue;
        strncpy(teams[i].name, token, MAX_TEAM_NAME_LEN - 1);
        teams[i].name[MAX_TEAM_NAME_LEN - 1] = '\0';

        token = strtok(NULL, ",");
        if (token != NULL) {
            teams[i].is_deleted = atoi(token);
        } else {
            teams[i].is_deleted = 0;
        }

        token = strtok(NULL, ",");
        if (token != NULL) {
            teams[i].is_hidden = atoi(token);
        } else {
            teams[i].is_hidden = 0;
        }
        
        teams[i].num_players = 0;
        i++;
    }
    fclose(f);
    *num_teams = i;

    // Load players
    FILE *pf = fopen(PLAYERS_FILE, "r");
    if (!pf) {
        return teams;
    }

    while (fgets(line, sizeof(line), pf)) {
        line[strcspn(line, "\r\n")] = 0;
        char *team_name = strtok(line, ",");
        if (team_name == NULL) continue;

        for (int j = 0; j < *num_teams; j++) {
            if (strcmp(teams[j].name, team_name) == 0) {
                if (teams[j].num_players < MAX_PLAYERS) {
                    Player *p = &teams[j].players[teams[j].num_players];
                    char *token;
                    int field = 0;
                    while((token = strtok(NULL, ",")) != NULL) {
                        switch(field) {
                            case 0: strncpy(p->name, token, MAX_PLAYER_NAME_LEN - 1); p->name[MAX_PLAYER_NAME_LEN - 1] = '\0'; break;
                            case 1: p->type = (PlayerType)atoi(token); break;
                            case 2: p->batting_style = (BattingStyle)atoi(token); break;
                            case 3: p->bowling_style = (BowlingStyle)atoi(token); break;
                            case 4: p->is_wicketkeeper = atoi(token); break;
                            case 5: p->is_active = atoi(token); break;
                            case 6: p->batting_skill = atoi(token); break;
                            case 7: p->bowling_skill = atoi(token); break;
                            case 8: p->fielding_skill = atoi(token); break;
                            case 9: p->matches_played = atoi(token); break;
                            case 10: p->total_runs = atoi(token); break;
                            case 11: p->total_wickets = atoi(token); break;
                            case 12: p->total_stumpings = atoi(token); break;
                            case 13: p->total_run_outs = atoi(token); break;
                        }
                        field++;
                    }
                    if (field == 14) {
                        teams[j].num_players++;
                    }
                }
                break;
            }
        }
    }
    fclose(pf);

    return teams;
}

void save_teams(const Team *teams, int num_teams) {
    FILE *f = fopen(TEAMS_FILE, "w");
    if (!f) return;
    FILE *pf = fopen(PLAYERS_FILE, "w");
    if (!pf) {
        fclose(f);
        return;
    }

    for (int i = 0; i < num_teams; i++) {
        fprintf(f, "%s,%d,%d\n", teams[i].name, teams[i].is_deleted, teams[i].is_hidden);
        for (int j = 0; j < teams[i].num_players; j++) {
            const Player *p = &teams[i].players[j];
            // New comprehensive fprintf for all player fields
            fprintf(pf, "%s,%s,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
                    teams[i].name, p->name, p->type, p->batting_style, p->bowling_style,
                    p->is_wicketkeeper, p->is_active, p->batting_skill, p->bowling_skill,
                    p->fielding_skill, p->matches_played, p->total_runs, p->total_wickets,
                    p->total_stumpings, p->total_run_outs);
        }
    }

    fclose(f);
    fclose(pf);
}


static void add_team_interactive(Team **teams, int *num_teams)
{
    char tname[64];
    printf("Enter new team name: ");
    getchar();
    if (!fgets(tname, sizeof(tname), stdin)) return;
    tname[strcspn(tname, "\r\n")] = 0;
    if (strlen(tname) == 0) { printf("Empty name, cancelled.\n"); return; }
    
    *teams = realloc(*teams, (*num_teams + 1) * sizeof(Team));
    Team *new_team = &(*teams)[*num_teams];
    strncpy(new_team->name, tname, MAX_TEAM_NAME_LEN - 1);
    new_team->name[MAX_TEAM_NAME_LEN - 1] = '\0';
    new_team->num_players = 0;
    new_team->is_hidden = false;

    printf("Team '%s' added. Now add players (up to 22).\n", tname);

    int num_global_players = 0;
    Player* global_players = load_global_players(&num_global_players);

    for (int i = 0; i < 22; ++i)
    {
        printf("Add player %d:\n1) Select from global list\n2) Add manually\nChoose (or 0 to stop): ", i+1);
        int choice;
        if (scanf("%d", &choice) != 1) { choice = 0; }
        clean_stdin();
        if (choice == 0) break;

        if (choice == 1 && global_players) {
            printf("Global Players:\n");
            for(int j=0; j<num_global_players; ++j) {
                printf("%d) %s\n", j+1, global_players[j].name);
            }
            printf("Select player: ");
            int p_idx;
            if (scanf("%d", &p_idx) != 1 || p_idx < 1 || p_idx > num_global_players) {
                printf("Invalid selection\n");
                i--; continue;
            }
            clean_stdin();
            new_team->players[new_team->num_players++] = global_players[p_idx-1];
        } else {
            Player p;
            printf("Add player name: ");
            if (!fgets(p.name, sizeof(p.name), stdin)) break;
            p.name[strcspn(p.name, "\r\n")] = 0;

            printf("Type: B=batsman, b=bowler, a=allrounder, w=wicketkeeper: ");
            char typec;
            if (scanf(" %c", &typec) != 1) { typec = 'B'; }
            clean_stdin();
            
            PlayerType t = PLAYER_TYPE_BATSMAN;
            if (typec == 'b') t = PLAYER_TYPE_BOWLER;
            else if (typec == 'a') t = PLAYER_TYPE_ALLROUNDER;
            else if (typec == 'w') t = PLAYER_TYPE_WICKETKEEPER;
            p.type = t;
            
            printf("Enter batting skill (1-100): ");
            if (scanf("%d", &p.batting_skill) != 1) p.batting_skill = 50;
            clean_stdin();
            printf("Enter bowling skill (1-100): ");
            if (scanf("%d", &p.bowling_skill) != 1) p.bowling_skill = 50;
            clean_stdin();
            
            // Set all career stats to 0 by default
            p.total_runs = 0;
            p.total_wickets = 0;
            p.total_stumpings = 0;
            p.total_run_outs = 0;
            p.matches_played = 0;
            new_team->players[new_team->num_players++] = p;
        }
    }
    
    (*num_teams)++;
    save_teams(*teams, *num_teams);
    free(global_players);
    printf("Players added for team '%s'.\n", tname);
}

static void list_teams(const Team *teams, int num_teams)
{
    if (num_teams == 0) {
        printf("No teams created yet.\n");
        return;
    }
    printf("Teams:\n");
    for (int i = 0; i < num_teams; i++) {
        printf(" - %s (%d players)\n", teams[i].name, teams[i].num_players);
    }
}


void show_player_stats(const Team *team) {
    printf("\n--- Player Stats for %s ---\n", team->name);
    printf("%-20s | %-15s | %-5s | %-5s | %-5s | %-5s\n", "Name", "Type", "Bat Skill", "Bowl Skill", "Runs", "Wickets");
    printf("--------------------------------------------------------------------------------\n");
    for (int i = 0; i < team->num_players; ++i) {
        const Player *p = &team->players[i];
        const char *type_str = "Batsman";
        if (p->type == PLAYER_TYPE_BOWLER) type_str = "Bowler";
        else if (p->type == PLAYER_TYPE_ALLROUNDER) type_str = "All-Rounder";
        else if (p->type == PLAYER_TYPE_WICKETKEEPER) type_str = "Wicket-Keeper";

        printf("%-20s | %-15s | %-9d | %-10d | %-5d | %-5d\n", 
               p->name, type_str, p->batting_skill, p->bowling_skill, p->total_runs, p->total_wickets);
    }
}

void initialize_dummy_teams(Team *teamA, Team *teamB) {
    strcpy(teamA->name, "Lions");
    teamA->num_players = 11;
    for (int i = 0; i < teamA->num_players; i++) {
        sprintf(teamA->players[i].name, "Lion %d", i + 1);
        teamA->players[i].type = (i < 5) ? PLAYER_TYPE_BATSMAN : (i < 10) ? PLAYER_TYPE_BOWLER : PLAYER_TYPE_WICKETKEEPER;
        teamA->players[i].batting_skill = 50 + (rand() % 40);
        teamA->players[i].bowling_skill = 20 + (rand() % 40);
        teamA->players[i].total_runs = 0;
        teamA->players[i].total_wickets = 0;
        teamA->players[i].total_stumpings = 0;
        teamA->players[i].total_run_outs = 0;
        teamA->players[i].matches_played = 0;
    }

    strcpy(teamB->name, "Tigers");
    teamB->num_players = 11;
    for (int i = 0; i < teamB->num_players; i++) {
        sprintf(teamB->players[i].name, "Tiger %d", i + 1);
        teamB->players[i].type = (i < 5) ? PLAYER_TYPE_BATSMAN : (i < 10) ? PLAYER_TYPE_BOWLER : PLAYER_TYPE_WICKETKEEPER;
        teamB->players[i].batting_skill = 50 + (rand() % 40);
        teamB->players[i].bowling_skill = 20 + (rand() % 40);
        teamB->players[i].total_runs = 0;
        teamB->players[i].total_wickets = 0;
        teamB->players[i].total_stumpings = 0;
        teamB->players[i].total_run_outs = 0;
        teamB->players[i].matches_played = 0;
    }
}


static void add_umpire()
{
    char name[64];
    printf("Enter umpire name: ");
    getchar();
    if (!fgets(name, sizeof(name), stdin)) return;
    name[strcspn(name, "\r\n")] = 0;
    if (strlen(name) == 0) { printf("Empty, cancelled.\n"); return; }
    FILE *f = fopen(UMPIRES_FILE, "a");
    if (!f) { printf("Unable to save umpire.\n"); return; }
    fprintf(f, "%s\n", name);
    fclose(f);
    printf("Umpire '%s' added.\n", name);
}

static void list_umpires()
{
    FILE *f = fopen(UMPIRES_FILE, "r");
    if (!f) { printf("No umpires added.\n"); return; }
    char line[128];
    printf("Umpires:\n");
    while (fgets(line, sizeof(line), f))
    {
        line[strcspn(line, "\r\n")] = 0;
        if (strlen(line)) printf(" - %s\n", line);
    }
    fclose(f);
}

Umpire* load_umpires(int *num_umpires) {
    FILE *f = fopen(UMPIRES_FILE, "r");
    if (!f) {
        *num_umpires = 0;
        return NULL;
    }

    // Count lines to allocate memory
    char line[256];
    int count = 0;
    while (fgets(line, sizeof(line), f)) {
        count++;
    }
    rewind(f);

    if (count == 0) {
        *num_umpires = 0;
        fclose(f);
        return NULL;
    }

    Umpire *umpires = (Umpire*)malloc(count * sizeof(Umpire));
    if (!umpires) {
        *num_umpires = 0;
        fclose(f);
        return NULL; // Allocation failed
    }

    int i = 0;
    while (fgets(line, sizeof(line), f) && i < count) {
        // New format: Name,Country,SinceYear,MatchesUmpired
        if (sscanf(line, "%63[^,],%63[^,],%d,%d", umpires[i].name, umpires[i].country, &umpires[i].since_year, &umpires[i].matches_umpired) == 4) {
            i++;
        }
    }

    *num_umpires = i;
    fclose(f);
    return umpires;
}

void save_umpires(const Umpire *umpires, int num_umpires) {
    FILE *f = fopen(UMPIRES_FILE, "w");
    if (!f) return;

    for (int i = 0; i < num_umpires; i++) {
        fprintf(f, "%s,%s,%d,%d\n", umpires[i].name, umpires[i].country, umpires[i].since_year, umpires[i].matches_umpired);
    }

    fclose(f);
}

void teams_menu(const char *user_email)
{
    (void)user_email; // Mark as unused to prevent warnings
    int num_teams = 0;
    Team *teams = load_teams(&num_teams);

    int running = 1;
    while (running)
    {
        const char *options[] = {"Create Team", "List Teams", "View Player Stats", "Back"};
        print_menu_box("Teams Menu", options, 4);
        printf("Choose: ");
        int c;
        if (scanf("%d", &c) != 1) { scanf("%*s"); clean_stdin(); continue; }
        clean_stdin();
        if (c == 1) {
            add_team_interactive(&teams, &num_teams);
        }
        else if (c == 2) {
            list_teams(teams, num_teams);
        }
        else if (c == 3) {
            if (num_teams == 0) {
                printf("No teams created yet.\n");
                continue;
            }
            printf("Select a team to view stats:\n");
            for (int i = 0; i < num_teams; i++) {
                printf("%d) %s\n", i + 1, teams[i].name);
            }
            int team_idx = -1;
            printf("Choose: ");
            if (scanf("%d", &team_idx) != 1) { scanf("%*s"); clean_stdin(); continue; }
            clean_stdin();
            if (team_idx > 0 && team_idx <= num_teams) {
                show_player_stats(&teams[team_idx - 1]);
            } else {
                printf("Invalid selection.\n");
            }
        }
        else if (c == 4) running = 0;
        else printf("Invalid choice.\n");
    }

    free(teams);
}

void umpires_menu(const char *user_email)
{
    (void)user_email; // Mark as unused to prevent warnings
    int running = 1;
    while (running)
    {
        const char *options[] = {"Add Umpire", "List Umpires", "Back"};
        print_menu_box("Umpires Menu", options, 3);
        printf("Choose: ");
        int c;
        if (scanf("%d", &c) != 1) { scanf("%*s"); clean_stdin(); continue; }
        clean_stdin();
        if (c == 1) add_umpire();
        else if (c == 2) list_umpires();
        else if (c == 3) running = 0;
        else printf("Invalid choice.\n");
    }
}
