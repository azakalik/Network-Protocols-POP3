#ifndef AVAILABLE_USERS
#define AVAILABLE_USERS

#define USERDATASIZE 512
#define MAXUSERS 500

#include <stdbool.h>
typedef struct {
    char name[USERDATASIZE];
    char password[USERDATASIZE];
} registered_users_data;

typedef struct {
    registered_users_data users[MAXUSERS];
    int userAmount;
} registered_users_singleton;

registered_users_singleton * createSingletonInstance(int userAmounts, char ** names);
bool validUsername(char * username);
bool validPassword(char * username, char * password);

#endif