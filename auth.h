
// Use define to prevent multiple inclusion
#ifndef AUTH_H
#define AUTH_H

// Authentication enum
// Valid types:
//  - AUTH_NONE
//  - AUTH_USERPASS
typedef enum auth_type {
    AUTH_NONE,
    AUTH_USERPASS
} auth_type;


// Create UserPass authentication struct
typedef struct {
    char *username;
    char *password;
} auth_userpass;

typedef union auth_data {
    auth_userpass *userpass;
} auth_data;

// Create authentication struct. This is the union of all authentication types.
typedef struct {
    auth_type type;
    auth_data *data;
} auth;

// Done
#endif // AUTH_H