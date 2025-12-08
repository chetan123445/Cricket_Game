#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "admin.h"
#include "accounts.h"
#include "teams.h"
#include "ui.h"
void manage_users_and_admins_menu();

static void list_all_users() {
    int num_accounts = 0;
    Account* accounts = load_all_accounts(&num_accounts);
    if (!accounts) {
        printf("No accounts found or failed to load.\n");
        return;
    }

    printf("\n--- All Users ---\n");
    printf("%-3s | %-30s | %-25s | %-10s\n", "ID", "Name", "Email", "Role");
    printf("------------------------------------------------------------------------------\n");
    for (int i = 0; i < num_accounts; i++) {
        const char* role = "User";
        if (is_superadmin(accounts[i].email)) {
            role = "Superadmin";
        } else if (accounts[i].isAdmin) {
            role = "Admin";
        }
        printf("%-3d | %-30s | %-25s | %-10s\n", i + 1, accounts[i].name, accounts[i].email, role);
    }
    printf("------------------------------------------------------------------------------\n");

    free(accounts);
}

static void add_new_user_or_admin() {
    int num_accounts = 0;
    Account* accounts = load_all_accounts(&num_accounts);

    Account new_acc;
    char isAdminChar;

    printf("Enter new user's name: ");
    clean_stdin();
    fgets(new_acc.name, sizeof(new_acc.name), stdin);
    new_acc.name[strcspn(new_acc.name, "\r\n")] = 0;

    printf("Enter new user's email: ");
    fgets(new_acc.email, sizeof(new_acc.email), stdin);
    new_acc.email[strcspn(new_acc.email, "\r\n")] = 0;

    // Check if email already exists
    for (int i = 0; i < num_accounts; i++) {
        if (strcmp(accounts[i].email, new_acc.email) == 0) {
            printf("Error: An account with this email already exists.\n");
            free(accounts);
            return;
        }
    }

    printf("Enter new user's password: ");
    fgets(new_acc.password, sizeof(new_acc.password), stdin);
    new_acc.password[strcspn(new_acc.password, "\r\n")] = 0;

    printf("Make this user an admin? (y/n): ");
    scanf(" %c", &isAdminChar);
    clean_stdin();
    new_acc.isAdmin = (isAdminChar == 'y' || isAdminChar == 'Y');

    num_accounts++;
    accounts = realloc(accounts, num_accounts * sizeof(Account));
    accounts[num_accounts - 1] = new_acc;

    save_all_accounts(accounts, num_accounts);
    printf("User '%s' added successfully.\n", new_acc.name);
    free(accounts);
}

static void edit_user() {
    list_all_users();
    int num_accounts = 0;
    Account* accounts = load_all_accounts(&num_accounts);
    if (!accounts) return;

    printf("Enter the ID of the user to edit: ");
    int id;
    if (scanf("%d", &id) != 1 || id < 1 || id > num_accounts) {
        printf("Invalid ID.\n");
        free(accounts);
        return;
    }
    clean_stdin();

    int index = id - 1;

    if (is_superadmin(accounts[index].email)) {
        printf("Cannot edit the superadmin.\n");
        free(accounts);
        return;
    }

    printf("Editing user: %s (%s)\n", accounts[index].name, accounts[index].email);
    printf("Enter new name (or press Enter to keep '%s'): ", accounts[index].name);
    char new_name[64];
    fgets(new_name, sizeof(new_name), stdin);
    new_name[strcspn(new_name, "\r\n")] = 0;
    if (strlen(new_name) > 0) {
        strcpy(accounts[index].name, new_name);
    }

    printf("Enter new password (or press Enter to keep current): ");
    char new_pass[128];
    fgets(new_pass, sizeof(new_pass), stdin);
    new_pass[strcspn(new_pass, "\r\n")] = 0;
    if (strlen(new_pass) > 0) {
        strcpy(accounts[index].password, new_pass);
    }

    printf("Make this user an admin? (current: %s) (y/n/Enter to keep): ", accounts[index].isAdmin ? "Yes" : "No");
    char choice;
    choice = getchar();
    if (choice != '\n') {
        clean_stdin();
        if (choice == 'y' || choice == 'Y') {
            accounts[index].isAdmin = true;
        } else if (choice == 'n' || choice == 'N') {
            accounts[index].isAdmin = false;
        }
    } else {
        // User pressed enter, do nothing
    }

    save_all_accounts(accounts, num_accounts);
    printf("User details updated.\n");
    free(accounts);
}


static void admin_add_team() {
    int num_teams = 0;
    Team *teams = load_teams(&num_teams);
    
    char tname[64];
    printf("Enter new team name: ");
    clean_stdin();
    if (!fgets(tname, sizeof(tname), stdin)) return;
    tname[strcspn(tname, "\r\n")] = 0;
    if (strlen(tname) == 0) { printf("Empty name, cancelled.\n"); return; }
    
    teams = realloc(teams, (num_teams + 1) * sizeof(Team));
    Team *new_team = &teams[num_teams];
    strncpy(new_team->name, tname, MAX_TEAM_NAME_LEN - 1);
    new_team->name[MAX_TEAM_NAME_LEN - 1] = '\0';
    new_team->num_players = 0;
    
    num_teams++;
    save_teams(teams, num_teams);
    free(teams);
    printf("Team '%s' added.\n", tname);
}

static void admin_add_player_to_pool() {
    Player p;
    
    printf("Add player name: ");
    clean_stdin();
    if (!fgets(p.name, sizeof(p.name), stdin)) { return; }
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
    
    p.runs_scored = 0;
    p.wickets_taken = 0;
    
    FILE *f = fopen("all_players.dat", "a");
    if (!f) {
        printf("Failed to open all_players.dat\n");
        return;
    }
    fprintf(f, "%s,%d,%d,%d\n", p.name, p.type, p.batting_skill, p.bowling_skill);
    fclose(f);

    printf("Player '%s' added to the global pool.\n", p.name);
}

static void admin_add_umpire()
{
    char name[64];
    printf("Enter umpire name: ");
    clean_stdin();
    if (!fgets(name, sizeof(name), stdin)) return;
    name[strcspn(name, "\r\n")] = 0;
    if (strlen(name) == 0) { printf("Empty, cancelled.\n"); return; }
    FILE *f = fopen("umpires.dat", "a");
    if (!f) { printf("Unable to save umpire.\n"); return; }
    fprintf(f, "%s\n", name);
    fclose(f);
    printf("Umpire '%s' added.\n", name);
}


void admin_menu(const char *user_email, const char *user_name) {
    int running = 1;
    while(running) {
        char title[128];
        sprintf(title, "Admin Menu for %s", user_name);        
        const char *options[] = {"Manage Admins and Users", "Add Team", "Add Player to Global Pool", "Add Umpire", "Back to Main Menu"};
        int num_options = 5;

        print_menu_box(title, options, num_options);
        printf("Choose: ");
        int c;
        if (scanf("%d", &c) != 1) { scanf("%*s"); clean_stdin(); continue; }
        clean_stdin();

        switch(c) {
            case 1:
                if (is_superadmin(user_email)) manage_users_and_admins_menu();
                else printf("You are not authorized to perform this action.\n");
                break;
            case 2:
                admin_add_team();
                break;
            case 3:
                admin_add_player_to_pool();
                break;
            case 4:
                admin_add_umpire();
                break;
            case 5:
                running = 0;
                break;
            default:
                printf("Invalid choice.\n");
        }
    }
}

void manage_users_and_admins_menu() {
    int running = 1;
    while(running) {
        const char *options[] = {"List All Users", "Add New User/Admin", "Edit User Details", "Back"};
        print_menu_box("User Management", options, 4);
        printf("Choose: ");
        int c;
        if (scanf("%d", &c) != 1) { scanf("%*s"); clean_stdin(); continue; }
        clean_stdin();

        switch(c) {
            case 1: list_all_users(); break;
            case 2: add_new_user_or_admin(); break;
            case 3: edit_user(); break;
            case 4: running = 0; break;
            default: printf("Invalid choice.\n");
        }
    }
}
