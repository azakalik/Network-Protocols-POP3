#include <stdlib.h>
#include <string.h>
#include "../server/serverFunctions.h"
#include <stdio.h>

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
} command_id;

typedef struct {
    char * commandStr;
    command_id execute_command;
} valid_command_list;

valid_command_list validCommands[TOTALCOMMANDS] = {
    {"TOP",TOP},{"USER",USER},{"PASS",PASS},{"STAT",STAT},{"LIST",LIST},{"RETR",RETR},
    {"DELE",DELE},{"NOOP",NOOP},{"RSET",RSET},{"QUIT",QUIT}
};


typedef struct {// strings are null terminated
    char buffer[MAXCOMMANDSIZE + 1];
    int currentBufferIdx;
} command_buffer;

typedef struct {// strings are null terminated
    char buffer[MAXARGSIZE + 1];
    int currentBufferIdx;
} arg_buffer;

typedef struct {
    command_buffer command;
    arg_buffer arg1;
    arg_buffer arg2;
    command_id execute_command;
    command_status commandStatus;
} full_command;

typedef struct {
    struct command_node *first;
    struct command_node *last;
} command_list;

typedef struct command_node {
    full_command data;
    struct command_node *next;
} command_node;

//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ AUXILIARY FUNCTIONS ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

//returns true on found command and sets command_id apropiately
//todo make more efficient
static bool checkValidCommand(full_command * full_command){
    bool commandFound = false;
    for ( int i = 0; !commandFound && i < TOTALCOMMANDS; i++){
        if ( strcmp(validCommands[i].commandStr, full_command->command.buffer) == 0 ){
            full_command->execute_command = validCommands[i].execute_command;
            commandFound = true;
        }
    }
    return commandFound;
   
}

static void processWritingCommand(full_command * full_command, char * data){
    if (data == NULL)
        return;

    bool finishedCommandParsing = false;

    int idx = full_command->command.currentBufferIdx;

    for (int i = 0; !finishedCommandParsing && idx + i < MAXCOMMANDSIZE; i++){
        if (data[i] == 0){
            finishedCommandParsing = true;
        } else if (data[i] == ' '){
            full_command->commandStatus = WRITINGARG1; //todo call processNode again
            finishedCommandParsing = true;
        } else if (data[i] == '\r' && data[i+1] == '\n'){
            full_command->commandStatus = COMPLETE;
            finishedCommandParsing = true;
        } else {
            full_command->command.buffer[idx + i] = data[i];
            full_command->command.currentBufferIdx = idx + i;
        }
    }
    if(finishedCommandParsing)
        checkValidCommand(full_command);
}


static void processNode(command_node * node, char * data){
    //todo hacer vector de punteros a funcion
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
    command_node * newNode = (command_node *) calloc(sizeof(command_node),1);
    newNode ->data.commandStatus = WRITINGCOMMAND;
    return newNode;
}

//↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ AUXILIARY FUNCTIONS ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑
//----------------------------------------------------------------------------------------------
//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ MAIN FUNCTIONS ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
command_list * createList(){
    command_list * list = (command_list *) calloc(sizeof(command_list), 1);
    return list;
}

bool addData(command_list *list, char * data) {
    if (list == NULL)
        return false;

    command_node * nodeToProcess;
    if ( list->first == NULL ) {
        command_node * newNode = createNewNode(); //todo check NULL
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
full_command * getFirstCommand(command_list * list){
    if (!availableCommands(list))
        return NULL;
    
    full_command * toReturn = (full_command *) list->first;

    if(list->first == list->last){
        list->last = list->first = NULL;
    } else {
        list->first = list->first->next;
    }
        
    return toReturn;
}

void freeCommand(command_list * list, full_command * command){ //todo fijarse si es necesario
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
//↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑ MAIN FUNCTIONS      ↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑↑