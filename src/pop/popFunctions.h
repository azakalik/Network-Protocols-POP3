#ifndef POP_FUNCTIONS
#define POP_FUNCTIONS

#include "popStandards.h"
#include "../clients/clients.h"
#include "../buffer/circularBuffer.h"

typedef int (*command_handler)(char * arg1, char * arg2, user_data * user_data); //pop functions declaration

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
int getUserMails(char * mailNo, char * empty, user_data * user_data);
int emptyFunction(char * arg1, char * empty, user_data * user_data);
int retr(char * msgNum, char * empty, user_data * user_data);
int signInWithUsername(char * username, char * empty, user_data * user_data);
int insertPassword(char * password, char * empty, user_data * user_data);

typedef enum {
    READINGFILE,
    READFIRSTCARRIAGE,
    READFIRSTNEWLINE,
    READDOT,
    READSECONDCARRIAGE,
    READNEWLINE
} stuffingStates;

#endif