#include <stdio.h>
#include <string.h>
#include "history.h"

static void make_history_filename(const char *email, char *out, size_t sz)
{
    size_t i, j=0;
    for (i=0; i<strlen(email) && j+1<sz; ++i)
    {
        char c = email[i];
        if (c=='@' || c=='.') out[j++] = '_';
        else out[j++] = c;
    }
    out[j] = 0;
    strncat(out, "_history.txt", sz - strlen(out) - 1);
}

void append_user_history(const char *user_email, const char *entry)
{
    char fname[256] = {0};
    make_history_filename(user_email, fname, sizeof(fname));
    FILE *f = fopen(fname, "a");
    if (!f) return;
    fprintf(f, "%s\n", entry);
    fclose(f);
}

void show_user_history(const char *user_email)
{
    char fname[256] = {0};
    make_history_filename(user_email, fname, sizeof(fname));
    FILE *f = fopen(fname, "r");
    if (!f) { printf("No history found for %s\n", user_email); return; }
    char line[1024];
    printf("\nHistory for %s:\n", user_email);
    while (fgets(line, sizeof(line), f)) printf("%s", line);
    fclose(f);
}
