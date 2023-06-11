#ifndef POP_FUNCTIONS
#define POP_FUNCTIONS

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../server/serverFunctions.h"
#include "../logger/logger.h"

typedef int (*command_handler)(char * arg1, char * arg2); //pop functions declaration

typedef struct valid_command_list {
    char * commandStr;
    command_handler execute_command;
} valid_command_list;

#define VALIDTHREELETTERSCOMMANDSIZE 1
#define VALIDFOURLETTERSCOMMANDSIZE 9
#define TOTALCOMMANDS VALIDTHREELETTERSCOMMANDSIZE + VALIDFOURLETTERSCOMMANDSIZE

command_handler getCommand(char * command_name);
void sendGreeting(user_data * user);
int getUserMails(char * username,user_buffer *userBuffer);
int emptyFunction(char * arg1, char * arg2);
int retr(char * username, char * msgNum, user_buffer *userBuffer);

typedef enum {
    AUTHENTICATION,
    TRANSACTION,
    UPDATE,
} pop_state;

typedef enum {
    READINGFILE,
    READFIRSTCARRIAGE,
    READFIRSTNEWLINE,
    READDOT,
    READSECONDCARRIAGE,
    READNEWLINE
} stuffingStates;

#endif