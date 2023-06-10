#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

#include "../commands/popFunctions.h"

typedef struct { //struct used to execute already commands
    char command[MAXCOMMANDSIZE+1];
    char arg1[MAXARGSIZE+1];
    char arg2[MAXARGSIZE+1];
    command_handler callback;
} command_to_execute;

//these are declared in commandParser.c
//we do not want code outside commandParser.c understand what is inside these structs
struct full_command;
struct command_list;

struct command_list * createList();

bool addData(struct command_list * list, char * data);

bool availableCommands(struct command_list * list);

command_to_execute * getFirstCommand(struct command_list * list);

void destroyList(struct command_list * list);

#endif