#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "accounts.h"
#include "tournament.h"
#include "teams.h"
#include "history.h"
#include "admin.h"
#include "ui.h"

#include "accounts.c"
#include "teams.c"
#include "match.c"
#include "tournament.c"
#include "history.c"
#include "admin.c"
#include "ui.c"

void main_menu(const char *email, const char *name);

int main()
{
    srand((unsigned)time(NULL));
    initialize_superadmin();
    printf("               Let's Play the Cricket Tournament (Modular)\n");
    printf("               ==========================================\n\n");

    UserType logged_in_user_type = USER_TYPE_NONE;
    char email[128] = {0};
    char name[64] = {0};

    while (1)
    {
        if (logged_in_user_type == USER_TYPE_NONE)
        {
            const char *options[] = {"Register", "Login", "Exit"};
            print_menu_box("Login Menu", options, 3);
            printf("Choose: ");
            int choice;
            if (scanf("%d", &choice) != 1) { scanf("%*s"); clean_stdin(); continue; }
            clean_stdin();
            if (choice == 1)
            {
                if (register_user()) printf("Registration done. You can login now.\n");
                else printf("Registration failed or cancelled.\n");
            }
            else if (choice == 2)
            {
                logged_in_user_type = login_user(email, name);
                if (logged_in_user_type != USER_TYPE_NONE)
                {
                    printf("Welcome %s (%s)\n", name, email);
                }
                else
                {
                    printf("Login failed.\n");
                }
            }
            else break;
        }
        else if (logged_in_user_type == USER_TYPE_ADMIN) {
            char title[128];
            sprintf(title, "Admin Access for %s", name);
            const char *options[] = {"Main Menu", "Admin Panel", "Logout"};
            print_menu_box(title, options, 3);
            printf("Choose: ");
            int c;
            if (scanf("%d", &c) != 1) { scanf("%*s"); clean_stdin(); continue; }
            clean_stdin();
            if (c == 1) {
                main_menu(email, name);
            } else if (c == 2) {
                admin_menu(email, name);
            } else if (c == 3) {
                memset(email, 0, sizeof(email));
                memset(name, 0, sizeof(name));
                logged_in_user_type = USER_TYPE_NONE;
                printf("Logged out.\n");
            } else {
                printf("Invalid choice.\n");
            }
        }
        else // Normal user
        {
            main_menu(email, name);
            logged_in_user_type = USER_TYPE_NONE; // after returning from main_menu, logout
        }
    }

    printf("Exiting. Goodbye.\n");
    return 0;
}

void main_menu(const char *email, const char *name) {
    int running = 1;
    while(running) {
        char title[128];
        sprintf(title, "Main Menu for %s", name);
        const char *options[] = {"Play Tournament / Matches", "Teams & Players (create/list)", "Add/View Umpires", "View Profile", "View My History", "Logout"};
        print_menu_box(title, options, 6);
        printf("Choose: ");
        int c;
        if (scanf("%d", &c) != 1) { scanf("%*s"); clean_stdin(); continue; }
        clean_stdin();
        if (c == 1)
        {
            tournament_menu(email, name);
        }
        else if (c == 2)
        {
            teams_menu(email);
        }
        else if (c == 3)
        {
            umpires_menu(email);
        }
        else if (c == 4)
        {
            printf("\nProfile\nName: %s\nEmail: %s\n", name, email);
        }
        else if (c == 5)
        {
            show_user_history(email);
        }
        else if (c == 6)
        {
            running = 0;
            printf("Logged out.\n");
        }
        else
        {
            printf("Invalid choice.\n");
        }
    }
}
