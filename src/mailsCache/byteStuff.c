#include "byteStuff.h"
#include "string.h"
#include <stdlib.h>

#define BUFFERSIZE 2048

typedef enum {
    NORMAL,
    READFIRSTCARRIAGE,
    READFIRSTNEWLINE,
    DOT,
    READSECONDCARRIAGE,
    INMEDIATERETURNCARRIAGE,
    INMEDIATERETURNNEWLINE,
} read_states;

typedef struct charactersProcessor {
    char buffer[BUFFERSIZE];
    int idx;
    int len;
    read_states state;
} charactersProcessor;

static void rebaseBuffer(charactersProcessor * charactersProcessor);
static int readFromBuffer(charactersProcessor * charactersProcessor);

charactersProcessor * initCharactersProcessor(){
    return calloc(1, sizeof(charactersProcessor));
}

void freeCharactersProcessor(charactersProcessor * charactersProcessor){
    free(charactersProcessor);
}

int getNProcessedCharacters(charactersProcessor * charactersProcessor, char * buffer, int n){
    if(charactersProcessor == NULL)
        return 0;
    int available = availableCharacters(charactersProcessor);
    int toCopy = available < n ? available : n;

    int copied;
    for (copied = 0; copied < toCopy; copied++)
    {
        int charRetrieved;
        if ((charRetrieved = readFromBuffer(charactersProcessor)) > 0)
            buffer[copied] = charRetrieved;
        else
            break;
    }
    

    // strncpy(buffer, charactersProcessor->buffer, toCopy);
    rebaseBuffer(charactersProcessor);
    return copied;
}

int addCharactersToProcess(charactersProcessor * charactersProcessor, char * buffer, int n){
    if(charactersProcessor == NULL)
        return 0;

    int freeSpace = availableSpace(charactersProcessor);
    int toAdd = freeSpace < n ? freeSpace : n;
    strncpy((charactersProcessor->buffer + charactersProcessor->len), buffer, toAdd);
    charactersProcessor->len += toAdd;
    return toAdd;
}

int availableCharacters(charactersProcessor * charactersProcessor){
    if(charactersProcessor == NULL)
        return 0;
    return charactersProcessor->len - charactersProcessor->idx;
}

int availableSpace(charactersProcessor * charactersProcessor){
    if(charactersProcessor == NULL)
        return 0;
    return BUFFERSIZE - availableCharacters(charactersProcessor);
}

void resetCharactersProcessor(charactersProcessor * charactersProcessor){
    charactersProcessor->idx = 0;
    charactersProcessor->len = 0;
    charactersProcessor->state = NORMAL;
}

static void rebaseBuffer(charactersProcessor * charactersProcessor){
    if ( charactersProcessor->len < charactersProcessor->idx){
        return;
    }

    int bytesToMove = availableCharacters(charactersProcessor);
    memmove(charactersProcessor->buffer, charactersProcessor->buffer + charactersProcessor->idx, bytesToMove );
    charactersProcessor->idx = 0;
    charactersProcessor->len = bytesToMove;
}

//returns amout of bytes read from buffer
static int readFromBuffer(charactersProcessor * charactersProcessor){
    //read but with state machine, EOF was not reached
    
    if ( charactersProcessor->idx >= charactersProcessor->len ){
        return 0;
    }

    int c = charactersProcessor->buffer[charactersProcessor->idx];


    if ( charactersProcessor->state == INMEDIATERETURNCARRIAGE){
        charactersProcessor->state = INMEDIATERETURNNEWLINE;
        return '\r';
    }

    if ( charactersProcessor->state == INMEDIATERETURNNEWLINE){
        charactersProcessor->state = NORMAL;
        return '\n';
    }

    if ( charactersProcessor->state == READSECONDCARRIAGE ){ 
        //tengo garantizado que en c tengo el que le sigue al '\r'
        if ( c == '\n'){
            charactersProcessor->state = INMEDIATERETURNCARRIAGE;
            charactersProcessor->idx++;
            return '.';
        }
    }

    if ( charactersProcessor->state == DOT){ 
        if ( charactersProcessor->idx + 1 < charactersProcessor->len){
            //es seguro preguntar por los 2 caracteres
            if ( c == '\r' && charactersProcessor->buffer[charactersProcessor->idx + 1] == '\n'){
                charactersProcessor->state = NORMAL;
                return '.';
            }
        } else if ( c == '\r'){
            charactersProcessor->state = READSECONDCARRIAGE;
            rebaseBuffer(charactersProcessor);
            charactersProcessor->idx++;
            return 0;
        }
    }

    if ( c == '\r'){
        charactersProcessor->state = READFIRSTCARRIAGE;
    } else if ( c == '\n'){
        if ( charactersProcessor->state == READFIRSTCARRIAGE){
            charactersProcessor->state = READFIRSTNEWLINE;
        }else{
            charactersProcessor->state = NORMAL;
        }
    } else if ( c == '.'){
        if (charactersProcessor->state == READFIRSTNEWLINE){
            charactersProcessor->state = DOT;
        }else {
            charactersProcessor->state = NORMAL;
        }
    } else {
        charactersProcessor->state = NORMAL;
    }

    charactersProcessor->idx++;
    return c;
}