#include <stdlib.h>
#include <string.h>
#include "../server/serverFunctions.h"
#include <stdio.h>
#include "commandParser.h"
#include "strings.h"


typedef enum command_status {    
    WRITINGCOMMAND, //the command isn't totally written, the next input from the user will be appended to command
    WRITINGARG1,
    WRITINGARG2,
    COMPLETE, //totally parsed complete command (ready to execute)
    INVALID, //when the command is detected as invalid but it isn't totally parsed yet
    COMPLETEINVALID, //when the command has been totally parsed and it is invalid
} command_status;

char statusNames[][16] = {"WRITINGCOMMAND", "WRITINGARG1", "WRITINGARG2", "COMPLETE", "INVALID", "COMPLETEINVALID"};

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

static bool isPrintableCharacter(char c){
    return c >= ' ' && c <= '~';
}

struct auxProcessWriting {
    command_status status_after_space;
    int maxSize;
    char * buffer;
    int * idx;
};

static void setupProcessWriting(struct auxProcessWriting * aux, full_command * command){
    if (command->commandStatus == WRITINGCOMMAND){
        aux->buffer = command->command.buffer;
        aux->idx = &command->command.currentBufferIdx;
        aux->status_after_space = WRITINGARG1;
        aux->maxSize = MAXCOMMANDSIZE;
    } else if (command->commandStatus == WRITINGARG1){
        aux->buffer = command->arg1.buffer;
        aux->idx = &command->arg1.currentBufferIdx;
        aux->status_after_space = WRITINGARG2;
        aux->maxSize = MAXARGSIZE;
    } else if (command->commandStatus == WRITINGARG2){
        aux->buffer = command->arg2.buffer;
        aux->idx = &command->arg2.currentBufferIdx;
        aux->status_after_space = INVALID;
        aux->maxSize = MAXARGSIZE;
    }
}

static int processWriting(full_command * full_command, char * data){
    bool finishedParsing = false;
    struct auxProcessWriting aux;

    setupProcessWriting(&aux, full_command);

    int i;
    for (i = 0; !finishedParsing; i++){
        if (data[i] == 0){
            finishedParsing = true;
        } else if (full_command->receivedCR){
            if (data[i] == '\n')
                full_command->commandStatus = COMPLETE;
            else
                full_command->commandStatus = INVALID;
        } else if (data[i] == ' '){
            full_command->commandStatus = aux.status_after_space;
            setupProcessWriting(&aux, full_command);
        } else if (data[i] == '\r'){
            full_command->receivedCR = true;
        } else if (*(aux.idx) >= aux.maxSize){
            full_command->commandStatus = INVALID;
            finishedParsing = true;
        } else if (isPrintableCharacter(data[i])){
            aux.buffer[*(aux.idx)] = data[i];
            *(aux.idx) += 1;
        } else {
            full_command->commandStatus = INVALID;
            finishedParsing = true;
        }
    }

    return i - 1;
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
    //if the command or the arguments weren't finished
    if (node->data.commandStatus == WRITINGCOMMAND || node->data.commandStatus == WRITINGARG1 || node->data.commandStatus == WRITINGARG2)
        charactersProcessed += processWriting(&node->data,data);

    //if an entire command has been parsed completely
    if (node->data.commandStatus == COMPLETE){
        command_handler command;
        if((command = getCommand(node->data.command.buffer)) != NULL){
            node->data.execute_command = command;
        } else {
            node->data.commandStatus = COMPLETEINVALID;
        }
    } else if (node->data.commandStatus == INVALID){ //when you know the command is invalid but haven't reached CRLF yet
        charactersProcessed += discardUntilCRLF(&node->data,data+charactersProcessed);
    }

    log(INFO, "Status: Command %s, Arg1 %s, Arg2 %s, Status %s", node->data.command.buffer, node->data.arg1.buffer, node->data.arg2.buffer, statusNames[node->data.commandStatus]);

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