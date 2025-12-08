#ifndef ACCOUNTS_H
#define ACCOUNTS_H

#include <stdbool.h>

// Define the UserType so other files like gui.c can use it
typedef enum {
    USER_TYPE_NONE,
    USER_TYPE_NORMAL,
    USER_TYPE_ADMIN
} UserType;

// Structure to hold a single user account's data in memory
typedef struct {
    char name[64];
    char email[128];
    char password[128];
    bool isAdmin;
} Account;
// --- Function Prototypes ---

// Initializes the superadmin account if it doesn't exist
void initialize_superadmin();

// Handles console-based user registration
int register_user();

// Handles console-based user login
UserType login_user(char *out_email, char *out_name);

// Handles GUI-based user login by reading from the accounts file
// This is the line that fixes the compilation error.
UserType login_user_from_file(const char *email, const char *password, char *out_name);

// Functions for the new admin panel
Account* load_all_accounts(int *num_accounts);
void save_all_accounts(const Account* accounts, int num_accounts);
bool is_superadmin(const char* email);

#endif // ACCOUNTS_H
