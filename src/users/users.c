#include "users.h"
#include <stdio.h>
#include <string.h>
#include "../logger/logger.h"


registered_users_singleton * createSingletonInstance(int userAmounts, char ** names){
    static bool initialized = false;
    static registered_users_singleton instance;
    if (initialized){
        return &instance;
    }
    char * delim = ":";
    for ( int i = 0; i < userAmounts ; i++){
        char * username = strtok(names[i],delim);
        char * password = strtok(NULL,delim);
        strcpy(instance.users[i].name,username);
        strcpy(instance.users[i].password,password);
    }
    instance.userAmount = userAmounts;
    initialized = true;
    return &instance;
}


static registered_users_singleton * getSingletonInstance(){
    static registered_users_singleton * instance = NULL;
    
    if (instance == NULL){
        instance = createSingletonInstance(0,NULL);
    }
    return instance;
}



bool validUsername(char * username){
    registered_users_singleton * singletonPtr = getSingletonInstance();
    bool found = false;
    for ( int i = 0; i < singletonPtr->userAmount && !found; i++){
        if (strcmp(singletonPtr->users[i].name,username) == 0){
            found = true;
        }
    }
    return found;
}


bool validPassword(char * username, char * password){
    registered_users_singleton * singletonPtr = getSingletonInstance();
    bool found = false;
    for ( int i = 0; i < singletonPtr->userAmount && !found; i++){
        if (strcmp(singletonPtr->users[i].password,password) == 0 && strcmp(singletonPtr->users[i].name,username) == 0){
            found = true;
        }
    }
    return found;
}
