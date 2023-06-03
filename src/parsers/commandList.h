typedef struct command_list command_list;
typedef struct command command;

command_list * createList();

bool addData(command_list * list, char * data);

bool availableCommands(command_list * list);

command * getFirstCommand(command_list * list);

void freeCommand(command_list * list, command * command);

void destroyList(command_list * list);