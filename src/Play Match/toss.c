#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "toss.h"
#include "ui.h"

void perform_toss(Team *teamA, Team *teamB, Team **batting_first, Team **fielding_first) {
    printf("\n--- Let's Play! ---\n");
    printf("%s vs %s\n", teamA->name, teamB->name);

    // Toss
    printf("Time for the toss. Heads (H) or Tails (T)? ");
    char toss_call;
    scanf(" %c", &toss_call);
    clean_stdin();
    int toss_outcome = rand() % 2; // 0 for heads, 1 for tails
    int user_won_toss = (toss_outcome == 0 && (toss_call == 'H' || toss_call == 'h')) || (toss_outcome == 1 && (toss_call == 'T' || toss_call == 't'));
    
    if (user_won_toss) {
        printf("You won the toss! Bat (1) or Field (2)? ");
        int choice;
        if (scanf("%d", &choice) != 1) choice = 1;
        clean_stdin();
        if (choice == 1) {
            printf("You chose to bat.\n");
            *batting_first = teamA;
            *fielding_first = teamB;
        } else {
            printf("You chose to field.\n");
            *batting_first = teamB;
            *fielding_first = teamA;
        }
    } else {
        printf("You lost the toss. The opponent will choose.\n");
        int opp_choice = rand() % 2;
        if (opp_choice == 0) {
            printf("Opponent chose to bat.\n");
            *batting_first = teamB;
            *fielding_first = teamA;
        } else {
            printf("Opponent chose to field.\n");
            *batting_first = teamA;
            *fielding_first = teamB;
        }
    }
}
