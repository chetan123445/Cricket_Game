#ifndef UMPIRES_H
#define UMPIRES_H

#define MAX_UMPIRE_NAME_LEN 100
#define MAX_COUNTRY_NAME_LEN 50

typedef struct {
    char name[MAX_UMPIRE_NAME_LEN];
    char country[MAX_COUNTRY_NAME_LEN];
    int since_year;
    int matches_umpired;
} Umpire;

Umpire* load_umpires(int* num_umpires);
void save_umpires(Umpire* umpires, int num_umpires);

#endif // UMPIRES_H
