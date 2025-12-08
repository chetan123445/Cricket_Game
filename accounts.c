#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "accounts.h"

static const char *ACCOUNTS_FILE = "accounts.dat";
static const char *ADMINS_FILE = "admins.dat";

static int email_exists(const char *email, char *out_name);

void initialize_superadmin() {
    FILE *f = fopen(ADMINS_FILE, "r");
    if (f) {
        fclose(f);
        return; // Already exists
    }

    f = fopen(ADMINS_FILE, "w");
    if (!f) return;
    fprintf(f, "chetandudi791@gmail.com\n");
    fclose(f);

    // Also register the superadmin as a user
    if (!email_exists("chetandudi791@gmail.com", NULL)) {
        FILE *acc = fopen(ACCOUNTS_FILE, "a");
        if (!acc) return;
        fprintf(acc, "chetandudi791@gmail.com,Jee@2022,Chetan Dudi\n");
        fclose(acc);
    }
}

static int is_admin(const char *email) {
    FILE *f = fopen(ADMINS_FILE, "r");
    if (!f) return 0;
    char line[256];
    while (fgets(line, sizeof(line), f)) {
        line[strcspn(line, "\r\n")] = 0;
        if (strcmp(line, email) == 0) {
            fclose(f);
            return 1;
        }
    }
    fclose(f);
    return 0;
}

bool is_superadmin(const char* email) {
    return (strcmp(email, "chetandudi791@gmail.com") == 0);
}

/* helper: check if email exists */
static int email_exists(const char *email, char *out_name)
{
    FILE *f = fopen(ACCOUNTS_FILE, "r");
    if (!f) return 0;
    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        char e[128], p[128], n[128];
        if (sscanf(line, "%127[^,],%127[^,],%127[^\n]", e, p, n) == 3)
        {
            if (strcmp(e, email) == 0)
            {
                if (out_name) strncpy(out_name, n, 127);
                fclose(f);
                return 1;
            }
        }
    }
    fclose(f);
    return 0;
}

int register_user()
{
    char email[128], pass[128], name[64];
    printf("Register - enter name: ");
    getchar();
    if (!fgets(name, sizeof(name), stdin)) return 0;
    name[strcspn(name, "\r\n")] = 0;
    printf("Email: ");
    if (!fgets(email, sizeof(email), stdin)) return 0;
    email[strcspn(email, "\r\n")] = 0;
    if (email_exists(email, NULL)) { printf("Email already registered.\n"); return 0; }
    printf("Password: ");
    if (!fgets(pass, sizeof(pass), stdin)) return 0;
    pass[strcspn(pass, "\r\n")] = 0;
    FILE *f = fopen(ACCOUNTS_FILE, "a");
    if (!f) { printf("Failed to open accounts file.\n"); return 0; }
    fprintf(f, "%s,%s,%s\n", email, pass, name);
    fclose(f);
    return 1;
}

UserType login_user(char *out_email, char *out_name)
{
    char email[128], pass[128];
    printf("Login - Email: ");
    if (scanf("%127s", email) != 1) return USER_TYPE_NONE;
    printf("Password: ");
    if (scanf("%127s", pass) != 1) return USER_TYPE_NONE;

    if (strcmp(email, "chetandudi791@gmail.com") == 0 && strcmp(pass, "Jee@2022") == 0) {
        strncpy(out_email, email, 127);
        strncpy(out_name, "Chetan Dudi (Superadmin)", 63);
        return USER_TYPE_ADMIN;
    }

    FILE *f = fopen(ACCOUNTS_FILE, "r");
    if (!f) { printf("No accounts exist. Please register first.\n"); return USER_TYPE_NONE; }
    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        char e[128], p[128], n[128];
        if (sscanf(line, "%127[^,],%127[^,],%127[^\n]", e, p, n) == 3)
        {
            if (strcmp(e, email) == 0 && strcmp(p, pass) == 0)
            {
                if (out_email) strncpy(out_email, e, 127);
                if (out_name) strncpy(out_name, n, 63);
                fclose(f);
                if (is_admin(email)) {
                    return USER_TYPE_ADMIN;
                }
                return USER_TYPE_NORMAL;
            }
        }
    }
    fclose(f);
    return USER_TYPE_NONE;
}

UserType login_user_from_file(const char *email, const char *password, char *out_name)
{
    // This special check is preserved to be consistent with the original console login function.
    if (strcmp(email, "chetandudi791@gmail.com") == 0 && strcmp(password, "Jee@2022") == 0) {
        if (out_name) strncpy(out_name, "Chetan Dudi (Superadmin)", 63);
        out_name[63] = '\0'; // Ensure null-termination
        return USER_TYPE_ADMIN;
    }

    FILE *f = fopen(ACCOUNTS_FILE, "r");
    if (!f) { 
        // If the file doesn't exist, no user can log in.
        return USER_TYPE_NONE; 
    }

    char line[512];
    while (fgets(line, sizeof(line), f))
    {
        char e[128], p[128], n[128];
        if (sscanf(line, "%127[^,],%127[^,],%127[^\n]", e, p, n) == 3)
        {
            if (strcmp(e, email) == 0 && strcmp(p, password) == 0)
            {
                if (out_name) strncpy(out_name, n, 63);
                fclose(f);
                return is_admin(email) ? USER_TYPE_ADMIN : USER_TYPE_NORMAL;
            }
        }
    }
    fclose(f);
    return USER_TYPE_NONE; // User not found or password incorrect
}

Account* load_all_accounts(int *num_accounts) {
    FILE *f = fopen(ACCOUNTS_FILE, "r");
    if (!f) {
        *num_accounts = 0;
        return NULL;
    }

    // Count lines to determine allocation size
    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), f)) count++;
    rewind(f);

    if (count == 0) {
        *num_accounts = 0;
        fclose(f);
        return NULL;
    }

    Account *accounts = (Account*)malloc(count * sizeof(Account));
    if (!accounts) {
        *num_accounts = 0;
        fclose(f);
        return NULL;
    }

    int i = 0;
    while (fgets(line, sizeof(line), f) && i < count) {
        if (sscanf(line, "%127[^,],%127[^,],%63[^\n]", accounts[i].email, accounts[i].password, accounts[i].name) == 3) {
            accounts[i].isAdmin = is_admin(accounts[i].email);
            i++;
        }
    }

    *num_accounts = i;
    fclose(f);
    return accounts;
}

void save_all_accounts(const Account* accounts, int num_accounts) {
    FILE *f_acc = fopen(ACCOUNTS_FILE, "w");
    FILE *f_adm = fopen(ADMINS_FILE, "w");

    if (!f_acc || !f_adm) {
        if (f_acc) fclose(f_acc);
        if (f_adm) fclose(f_adm);
        return;
    }

    // Always ensure the superadmin is in the admins file
    fprintf(f_adm, "chetandudi791@gmail.com\n");

    for (int i = 0; i < num_accounts; i++) {
        // Save account to accounts.dat
        fprintf(f_acc, "%s,%s,%s\n", accounts[i].email, accounts[i].password, accounts[i].name);

        // If they are an admin (and not the superadmin, who is already saved), add to admins.dat
        if (accounts[i].isAdmin && !is_superadmin(accounts[i].email)) {
            fprintf(f_adm, "%s\n", accounts[i].email);
        }
    }

    fclose(f_acc);
    fclose(f_adm);
}
