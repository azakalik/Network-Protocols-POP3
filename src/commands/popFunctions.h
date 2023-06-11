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
#include "popStates.h"

typedef int (*command_handler)(char * arg1, char * arg2); //pop functions declaration

typedef struct command_with_state {
    char * commandStr;
    command_handler execute_command;
    pop_state pop_state; //pop state needed to execute this command
} command_with_state;

#define VALIDTHREELETTERSCOMMANDSIZE 1
#define VALIDFOURLETTERSCOMMANDSIZE 9
#define TOTALCOMMANDS VALIDTHREELETTERSCOMMANDSIZE + VALIDFOURLETTERSCOMMANDSIZE

command_with_state * getCommand(char * command_name);
void sendGreeting(user_data * user);
int getUserMails(char * username,user_buffer *userBuffer);
int emptyFunction(char * arg1, char * arg2);
int retr(char * username, char * msgNum, user_buffer *userBuffer);

typedef enum {
    READINGFILE,
    READFIRSTCARRIAGE,
    READFIRSTNEWLINE,
    READDOT,
    READSECONDCARRIAGE,
    READNEWLINE
} stuffingStates;

#endif