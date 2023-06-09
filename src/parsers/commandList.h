#ifndef COMMAND_LIST_H
#define COMMAND_LIST_H

typedef int (*command_handler)(char * arg1, char * arg2); //pop functions declaration

typedef struct {
    char command[MAXCOMMANDSIZE+1];
    char arg1[MAXARGSIZE+1];
    char arg2[MAXARGSIZE+1];
    command_handler callback;
} command_to_execute;

struct full_command;
struct command_list;


struct command_list * createList();

bool addData(struct command_list * list, char * data);

bool availableCommands(struct command_list * list);

command_to_execute * getFirstCommand(struct command_list * list);

void freeCommand(struct command_list * list, struct full_command * command);

void destroyList(struct command_list * list);

#endif