#include <stdlib.h>
#include <string.h>
#include "../server/serverFunctions.h"
#include <stdio.h>
#include "../commands/popFunctions.h"
#include "commandList.h"
#include "strings.h"

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

typedef enum command_status {    
    WRITINGCOMMAND,
    WRITINGARG1,
    WRITINGARG2,
    COMPLETE, //totally parsed complete command (ready to execute)
    INVALID, //when the command is detected as invalid but it isn't totally parsed yet
    COMPLETEINVALID, //when the command has been totally parsed and it is invalid
} command_status;

char statusNames[6][16] = {"WRITINGCOMMAND", "WRITINGARG1", "WRITINGARG2", "COMPLETE", "INVALID", "COMPLETEINVALID"};

#define VALIDTHREELETTERSCOMMANDSIZE 1
#define VALIDFOURLETTERSCOMMANDSIZE 9
#define TOTALCOMMANDS VALIDTHREELETTERSCOMMANDSIZE + VALIDFOURLETTERSCOMMANDSIZE


typedef struct valid_command_list {
    char * commandStr;
    command_handler execute_command;
} valid_command_list;

valid_command_list validCommands[TOTALCOMMANDS] = {
    {"TOP",emptyFunction},{"USER",emptyFunction},{"PASS",emptyFunction},{"STAT",emptyFunction},{"LIST",emptyFunction},{"RETR",emptyFunction},
    {"DELE",emptyFunction},{"NOOP",emptyFunction},{"RSET",emptyFunction},{"QUIT",emptyFunction}
};


typedef struct command_buffer {// strings are null terminated
    char buffer[MAXCOMMANDSIZE + 1];
    int currentBufferIdx;
} command_buffer;

typedef struct arg_buffer {// strings are null terminated
    char buffer[MAXARGSIZE + 1];
    int currentBufferIdx;
} arg_buffer;

typedef struct full_command {
    command_buffer command;
    arg_buffer arg1;
    arg_buffer arg2;
    command_handler execute_command;
    command_status commandStatus;
    bool receivedCR;
} full_command;

typedef struct command_list {
    struct command_node *first;
    struct command_node *last;
} command_list;

typedef struct command_node {
    full_command data;
    struct command_node *next;
} command_node;

//↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ AUXILIARY FUNCTIONS ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓

//returns true on found command and sets execute_command apropiately
//todo make more efficient
static void checkValidCommand(full_command * full_command){
    bool commandFound = false;
    for ( int i = 0; !commandFound && i < TOTALCOMMANDS; i++){
        if ( strcasecmp(validCommands[i].commandStr, full_command->command.buffer) == 0 ){
            full_command->execute_command = validCommands[i].execute_command;
            commandFound = true;
        }
    }
    if(!commandFound){
        if(full_command->commandStatus == COMPLETE)
            full_command->commandStatus = COMPLETEINVALID;
        else
            full_command->commandStatus = INVALID;
    }
    return;
   
}

static bool isPrintableCharacter(char c){
    return c >= ' ' && c <= '~';
}

static int processWriting(full_command * full_command, char * data){
    bool finishedParsing = false;

    int idx;
    command_status status_after_space;
    int maxSize = MAXARGSIZE;
    if (full_command->commandStatus == WRITINGCOMMAND){
        idx = full_command->command.currentBufferIdx;
        status_after_space = WRITINGARG1;
        maxSize = MAXCOMMANDSIZE;
    } else if (full_command->commandStatus == WRITINGARG1){
        idx = full_command->arg1.currentBufferIdx;
        status_after_space = WRITINGARG2;
    } else if (full_command->commandStatus == WRITINGARG2){
        idx = full_command->arg2.currentBufferIdx;
        status_after_space = INVALID;
    }

    int i;
    for (i = 0; !finishedParsing; i++){
        if (data[i] == 0){
            finishedParsing = true;
        } else if (full_command->receivedCR){
            if (data[i] == '\n')
                full_command->commandStatus = COMPLETE;
            else
                full_command->commandStatus = INVALID;
            finishedParsing = true;
        } else if (data[i] == ' '){
            full_command->commandStatus = status_after_space;
            finishedParsing = true;
        } else if (data[i] == '\r'){
            full_command->receivedCR = true;
        } else if (idx + i >= maxSize){
            full_command->commandStatus = INVALID;
            finishedParsing = true;
        } else if (isPrintableCharacter(data[i])){
            if (full_command->commandStatus == WRITINGCOMMAND){
                full_command->command.buffer[idx + i] = data[i];
                full_command->command.currentBufferIdx += 1;
            } else if(full_command->commandStatus == WRITINGARG1){
                full_command->arg1.buffer[idx + i] = data[i];
                full_command->arg1.currentBufferIdx += 1;
            } else if(full_command->commandStatus == WRITINGARG2){
                full_command->arg2.buffer[idx + i] = data[i];
                full_command->arg2.currentBufferIdx += 1;
            }
        } else {
            full_command->commandStatus = INVALID;
            finishedParsing = true;
        }
    }

    return i;
}

static int discardUntilCRLF(full_command * full_command, char * data){
    bool finishedParsing = false;

    int i;
    for (i = 0; !finishedParsing; i++){
        if (data[i] == 0){
            finishedParsing = true;
        } else if (data[i] == '\r'){
            full_command->receivedCR = true;
        } else if (data[i] == '\n' && full_command->receivedCR) {
            full_command->commandStatus = COMPLETEINVALID;
            finishedParsing = true;
        } else {
            full_command->receivedCR = false;
        }
    }

    return i;
}

static int processNode(command_node * node, char * data){ //todo que pasa cuando data tiene mas de un comando
    if (data == NULL)
        return 0;

    int charactersProcessed = 0;
    //todo hacer vector de punteros a funcion
    if (node->data.commandStatus == WRITINGCOMMAND)
        charactersProcessed += processWriting(&node->data,data);

    if (node->data.commandStatus == WRITINGARG1)
        charactersProcessed += processWriting(&node->data,data+charactersProcessed);

    if (node->data.commandStatus == WRITINGARG2)
        charactersProcessed += processWriting(&node->data,data+charactersProcessed);

    if (node->data.commandStatus == COMPLETE)
        checkValidCommand(&node->data);

    if (node->data.commandStatus == INVALID)
        charactersProcessed += discardUntilCRLF(&node->data,data+charactersProcessed);

    log(INFO, "Status: Command %s, Arg1 %s, Arg2 %s", node->data.command.buffer, node->data.arg1.buffer, node->data.arg2.buffer);
    log(INFO, "The previous command has status %s", statusNames[node->data.commandStatus]);

    

    return charactersProcessed;
}

static command_node * createNewNode(){
    command_node * newNode = (command_node *) calloc(sizeof(command_node),1);
    newNode ->data.commandStatus = WRITINGCOMMAND;
    return newNode;
}

static bool isEmpty(command_list * list){
    return list->first == NULL;
}

//creates a node and adds it to the last position of the list. returns the node created
static command_node * addNodeToList(command_list * list){
    command_node * newNode = createNewNode();
    if ( isEmpty(list) ) {
        list->first = newNode;
        list->last = newNode;
    } else {
        list->last->next = newNode;
        list->last = list->last->next;
    }
    return newNode;
}

static void deleteFirstNode(command_list * list){
    if(list == NULL || isEmpty(list))
        return;

    command_node * toDelete = list->first;
    if (list->first == list->last)
        list->first = list->last = NULL;
    else {
        list->first = list->first->next;
    }

    free(toDelete);
}


static void deleteLastNode(command_list * list){
    if(list == NULL || isEmpty(list))
        return;

    command_node * toDelete = list->last;
    if (list->first == list->last)
        list->first = list->last = NULL;
    else {
        command_node * node = list->first;
        while(node->next->next != NULL){
            node = node->next;
        }
        list->last = node;
    }

    free(toDelete);
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

    log(INFO, "New stream received, processing it...");

    int charactersProcessed = 0;
    while(charactersProcessed != MAXLINESIZE+1 && data[charactersProcessed] != 0){
        command_node * nodeToProcess;
        if ( isEmpty(list) || list->last->data.commandStatus == COMPLETE ) {
            nodeToProcess = addNodeToList(list);
        } else if ( list->last->data.commandStatus == COMPLETEINVALID ){
            log(ERROR, "Invalid command");
            deleteLastNode(list);
            nodeToProcess = addNodeToList(list);
        } else { //last was in WRITING mode
            nodeToProcess = list->last;
        }

        charactersProcessed += processNode(nodeToProcess, data+charactersProcessed);
    }
    
    return true;
}

//removes INVALIDCOMPLETE nodes until it finds a COMPLETE node
//returns true if the remaining first node is COMPLETE
bool availableCommands(command_list * list){
    if (list == NULL || list->first == NULL)
        return false;

    while(!isEmpty(list) && list->first->data.commandStatus == COMPLETEINVALID){
        deleteFirstNode(list);
    }

    return isEmpty(list) ? false : list->first->data.commandStatus == COMPLETE;
}

//returns null if there is no COMPLETE command to return
//returns the first struct command if there is a COMPLETE one to return. it then removes it from the list
//the user HAS to free the node returned
command_to_execute * getFirstCommand(command_list * list){
    if (!availableCommands(list))
        return NULL;
    
    full_command * toTransform = (full_command *) list->first;

    command_to_execute * toReturn = calloc(1, sizeof(command_to_execute));
    strcpy(toReturn->command, toTransform->command.buffer);
    strcpy(toReturn->arg1, toTransform->arg1.buffer);
    strcpy(toReturn->arg2, toTransform->arg2.buffer);
    toReturn->callback = toTransform->execute_command;

    deleteFirstNode(list);
        
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