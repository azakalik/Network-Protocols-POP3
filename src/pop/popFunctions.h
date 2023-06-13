#ifndef POP_FUNCTIONS
#define POP_FUNCTIONS

#include "popStandards.h"
#include "../clients/clients.h"

#define COMMANDCOMPLETED 0
#define INCOMPLETECOMMAND 1
#define COMMANDERROR 2

typedef int (*command_handler)(char * arg1, char * arg2, user_data * user_data); //pop functions declaration

typedef struct command_with_state {
    char * commandStr;
    command_handler execute_command;
    pop_state pop_state_needed; //pop state needed to execute this command
} command_with_state;

#define VALIDTHREELETTERSCOMMANDSIZE 1
#define VALIDFOURLETTERSCOMMANDSIZE 9
#define TOTALCOMMANDS VALIDTHREELETTERSCOMMANDSIZE + VALIDFOURLETTERSCOMMANDSIZE

command_with_state * getCommand(char * command_name);
int sendGreeting(user_data * user);

typedef enum {
    READINGFILE,
    READFIRSTCARRIAGE,
    READFIRSTNEWLINE,
    READDOT,
    READSECONDCARRIAGE,
    READNEWLINE
} stuffingStates;

#endif