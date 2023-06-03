#include <stdlib.h>
#include <string.h>
#include "serverFunctions.h"
#include <stdio.h>

// primer nodo (para consumir comandos completos)
// ultimo nodo (para ver si tiene que seguir escribiendo ahi)
// agregar un nodo al final
// borrar un nodo del principio

/*
STAT Command ................................................    6
LIST Command ................................................    6
RETR Command ................................................    8
DELE Command ................................................    8
NOOP Command ................................................    9
RSET Command ................................................    9
The UPDATE State ............................................   10
QUIT Command ................................................   10
*/



typedef enum{    
    WRITINGCOMMAND,
    WRITINGARG1,
    WRITINGARG2,
    COMPLETE,
} command_status;

#define VALIDTHREELETTERSCOMMANDSIZE 1
#define VALIDFOURLETTERSCOMMANDSIZE 9
#define TOTALCOMMANDS VALIDTHREELETTERSCOMMANDSIZE + VALIDFOURLETTERSCOMMANDSIZE
#define THREELENGTHCOMMAND 3
#define FOURLENGTHCOMMAND 4
#define VALIDCOMMANDSLENGTH 10


typedef enum {
    USER,
    PASS,
    STAT,
    LIST,
    RETR,
    DELE,
    NOOP,
    RSET,
    QUIT,
    TOP
} to_execute_command;

typedef struct {
    char * commandStr;
    to_execute_command commandName;
} valid_command_list;

valid_command_list validCommands[TOTALCOMMANDS] = {
    {"TOP",TOP},{"USER",USER},{"PASS",PASS},{"STAT",STAT},{"LIST",LIST},{"RETR",RETR},
    {"DELE",DELE},{"NOOP",NOOP},{"RSET",RSET},{"QUIT",QUIT}
};


typedef struct {
    char buffer[MAXCOMMANDSIZE + 1];
    int currentBufferIdx;
} command_buffer;

typedef struct {
    char buffer[MAXARGSIZE + 1];
    int currentBufferIdx;
} arg_buffer;



typedef struct {
    command_buffer command;
    arg_buffer arg1;
    arg_buffer arg2;
    to_execute_command executeCommandName;
} command_data;





// strings are null terminated
typedef struct{
    command_data commandData;
    command_status commandStatus;
} command;



typedef struct {
    struct command_node *first;
    struct command_node *last;
} command_list;


typedef struct command_node {
    command data;
    struct command_node *next;
} command_node;


command_list * createList(){
    command_list * list = calloc(sizeof(command_list), 1);
    return list;
}

//returns true on found command and sets to_execute_command apropiately
static bool checkValidCommand(command_buffer * command, to_execute_command * commandName){
    bool commandFound = false;
    for ( int i = 0;!commandFound && i < VALIDCOMMANDSLENGTH ; i++){
        if ( strcmp(validCommands[i].commandStr,command->buffer)){
            *commandName = validCommands[i].commandName;
            commandFound = true;
        }
    }
    return commandFound;
   
}

static void processWritingCommand(command * command, char * data){

    command_data* commandData = &command->commandData;
    bool finishedCommandParsing = false;
    for ( int i = 0; commandData->command.currentBufferIdx < MAXCOMMANDSIZE && data[i] != 0 && !finishedCommandParsing; ){
        if ( commandData->command.currentBufferIdx == THREELENGTHCOMMAND || commandData->command.currentBufferIdx == FOURLENGTHCOMMAND){
            finishedCommandParsing = checkValidCommand(&commandData->command,&commandData->executeCommandName);
            if ( finishedCommandParsing ){
                //TODO VER SI PODEMOS DEFINIR ACA QUE EL COMANDO TERMINO
                command->commandStatus = WRITINGARG1;
            }
        }
    }

    

}


static void processNode(command_node * node, char * data){

    switch (node->data.commandStatus)
    {
    case WRITINGCOMMAND:
        processWritingCommand(&node->data,data);
        break;
    case WRITINGARG1:
    case WRITINGARG2:

        break;
    default:
        break;
    }
    

}

static command_node * createNewNode(){
    command_node * newNode = calloc(sizeof(command_node),1);
    newNode ->data.commandStatus = WRITINGCOMMAND;
    return newNode;
}

bool addData(command_list *list, char * data) {
    if (list == NULL)
        return false;

    command_node * nodeToProcess;
    if ( list->first == NULL ) {
        command_node * newNode = createNewNode();
        list->first = list->last = newNode;
        nodeToProcess = newNode;
    } else if ( list->last->data.commandStatus == COMPLETE) {
        command_node * newNode = createNewNode();
        list->last->next = newNode;
        list->last = newNode;
        nodeToProcess = newNode;
    } else { //last was in WRITING mode
        nodeToProcess = list->last;
    }

    processNode(nodeToProcess, data);
    return true;
}

//returns true if there is at least one COMPLETE command to get
bool availableCommands(command_list * list){
    if (list == NULL || list->first == NULL)
        return false;

    return list->first->data.commandStatus == COMPLETE;
}

//returns null if there is no COMPLETE command to return
//returns the first struct command if there is a COMPLETE one to return. it then removes it from the list
//the user HAS to free it
command * getFirstCommand(command_list * list){
    if (!availableCommands(list))
        return NULL;
    
    command * toReturn = (command *) list->first;

    if(list->first == list->last){
        list->last = list->first = NULL;
    } else {
        list->first = list->first->next;
    }
        
    return toReturn;
}

void freeCommand(command_list * list, command * command){ //todo fijarse si es necesario
    free( (command_node *) command );
}

void destroyList(command_list * list) {
    if (list == NULL)
        return;
    
    command_node * node = list->first;
    command_node * nodeToFree;
    
    while(node != NULL){
        nodeToFree = node;
        node = node->next;
        free(nodeToFree);
    }
    
    free(list);
}
