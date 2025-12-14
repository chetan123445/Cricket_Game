#ifndef GROUNDS_H
#define GROUNDS_H

#define MAX_GROUND_NAME_LEN 100
#define MAX_COUNTRY_NAME_LEN 50

typedef struct {
    char name[MAX_GROUND_NAME_LEN];
    char country[MAX_COUNTRY_NAME_LEN];
} Ground;

Ground* load_grounds(int* num_grounds);
void save_grounds(Ground* grounds, int num_grounds);

#endif // GROUNDS_H
