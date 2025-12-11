#include <stdio.h>
#include <string.h>
#include "ui.h"

void clean_stdin() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void print_menu_box(const char *title, const char *options[], int num_options) {
    int max_len = strlen(title);
    for (int i = 0; i < num_options; i++) {
        // Accommodate for "i+1) "
        int len = strlen(options[i]) + 4;
        if (len > max_len) {
            max_len = len;
        }
    }

    int box_width = max_len + 4;
    int terminal_width = 80; // Assuming a standard terminal width
    int padding = (terminal_width - box_width) / 2;
    if (padding < 0) padding = 0;

    char pad_str[128];
    for (int i=0; i<padding; ++i) pad_str[i] = ' ';
    pad_str[padding] = '\0';

    // Top border
    printf("\n%s%c", pad_str, 201); // ╔
    for (int i = 0; i < box_width; i++) printf("%c", 205); // ═
    printf("%c\n", 187); // ╗

    // Title
    printf("%s%c %s", pad_str, 186, title); // ║
    for (size_t i = 0; i < box_width - strlen(title) - 1; i++) printf(" ");
    printf("%c\n", 186); // ║

    // Separator
    printf("%s%c", pad_str, 204); // ╠
    for (int i = 0; i < box_width; i++) printf("%c", 205); // ═
    printf("%c\n", 185); // ╣

    // Options
    for (int i = 0; i < num_options; i++) {
        char option_str[256];
        sprintf(option_str, "%d) %s", i + 1, options[i]);
        printf("%s%c %s", pad_str, 186, option_str); // ║
        for (size_t j = 0; j < box_width - strlen(option_str) - 1; j++) printf(" ");
        printf("%c\n", 186); // ║
    }

    // Bottom border
    printf("%s%c", pad_str, 200); // ╚
    for (int i = 0; i < box_width; i++) printf("%c", 205); // ═
    printf("%c\n", 188); // ╝
}
