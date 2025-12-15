#include "umpires.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

Umpire* load_umpires(int* num_umpires) {
    FILE* file = fopen("Data/umpires.dat", "r");
    if (!file) {
        *num_umpires = 0;
        return NULL;
    }

    Umpire* umpires = NULL;
    char line[256];
    int count = 0;

    while (fgets(line, sizeof(line), file)) {
        line[strcspn(line, "\r\n")] = 0; // Remove newline
        umpires = realloc(umpires, (count + 1) * sizeof(Umpire));
        
        char *token;
        token = strtok(line, ",");
        if(token == NULL) continue;
        strncpy(umpires[count].name, token, MAX_UMPIRE_NAME_LEN - 1);
        umpires[count].name[MAX_UMPIRE_NAME_LEN - 1] = '\0';

        token = strtok(NULL, ",");
        if(token == NULL) continue;
        strncpy(umpires[count].country, token, MAX_COUNTRY_NAME_LEN - 1);
        umpires[count].country[MAX_COUNTRY_NAME_LEN - 1] = '\0';

        token = strtok(NULL, ",");
        if(token == NULL) continue;
        umpires[count].since_year = atoi(token);
        
        token = strtok(NULL, ",");
        if(token == NULL) continue;
        umpires[count].matches_umpired = atoi(token);

        count++;
    }

    *num_umpires = count;
    fclose(file);
    return umpires;
}

void save_umpires(Umpire* umpires, int num_umpires) {
    FILE* file = fopen("Data/umpires.dat", "w");
    if (!file) {
        return;
    }

    for (int i = 0; i < num_umpires; i++) {
        fprintf(file, "%s,%s,%d,%d\n", umpires[i].name, umpires[i].country, umpires[i].since_year, umpires[i].matches_umpired);
    }

    fclose(file);
}

