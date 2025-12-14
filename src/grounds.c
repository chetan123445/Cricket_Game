#include "grounds.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

Ground* load_grounds(int* num_grounds) {
    FILE* file = fopen("Data/grounds.dat", "rb");
    if (!file) {
        *num_grounds = 0;
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size == 0) {
        *num_grounds = 0;
        fclose(file);
        return NULL;
    }

    *num_grounds = file_size / sizeof(Ground);
    Ground* grounds = (Ground*)malloc(file_size);
    if (!grounds) {
        *num_grounds = 0;
        fclose(file);
        return NULL;
    }

    fread(grounds, sizeof(Ground), *num_grounds, file);
    fclose(file);

    return grounds;
}

void save_grounds(Ground* grounds, int num_grounds) {
    FILE* file = fopen("Data/grounds.dat", "wb");
    if (!file) {
        return;
    }

    fwrite(grounds, sizeof(Ground), num_grounds, file);
    fclose(file);
}