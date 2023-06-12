#include "users.h"
#include <stdio.h>
#include <string.h>
#define MAXUSERS 10


static registered_users_singleton * createSingletonInstance(){
    static registered_users_singleton instance;
    strcpy(instance.users[0].name,"sranucci");
    strcpy(instance.users[0].password,"protos");
    instance.userAmount = 1;
    return &instance;
}


static registered_users_singleton * getSingletonInstance(){
    static registered_users_singleton * instance = NULL;
    if (instance == NULL){
        instance = createSingletonInstance();
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
