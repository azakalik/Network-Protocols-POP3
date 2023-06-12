#ifndef AVAILABLE_USERS
#define AVAILABLE_USERS

#define USERDATASIZE 512
#define MAXUSERS 10

#include <stdbool.h>
typedef struct {
    char name[USERDATASIZE];
    char password[USERDATASIZE];
} registered_users_data;

typedef struct {
    registered_users_data users[MAXUSERS];
    int userAmount;
} registered_users_singleton;

bool validUsername(char * username);
bool validPassword(char * password);

#endif