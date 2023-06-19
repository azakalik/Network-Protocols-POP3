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

    strncpy(buffer, charactersProcessor->buffer, toCopy);
    charactersProcessor->idx += toCopy;

    rebaseBuffer(charactersProcessor);
    return toCopy;
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

static void rebaseBuffer(charactersProcessor * charactersProcessor){
    if ( charactersProcessor->len < charactersProcessor->idx){
        return;
    }

    int bytesToMove = availableCharacters(charactersProcessor);
    memmove(charactersProcessor->buffer, charactersProcessor->buffer + charactersProcessor->idx, bytesToMove );
    charactersProcessor->idx = 0;
    charactersProcessor->len = bytesToMove;
}

// //returns amout of bytes read from buffer
// //todo falla cuando en el final del archivo hay un \r\n.\r
// static int readFromBuffer(charactersProcessor * charactersProcessor){
//     //read but with state machine, EOF was not reached
    
//     if ( buffer->currentPos >= buffer->len ){
//         return NOAVAILABLECONTENT;
//     }

//     int c = buffer->auxBuffer[buffer->currentPos];


//     if ( buffer->state == INMEDIATERETURNCARRIAGE){
//         buffer->state = INMEDIATERETURNNEWLINE;
//         return '\r';
//     }

//     if ( buffer->state == INMEDIATERETURNNEWLINE){
//         buffer->state = NORMAL;
//         return '\n';
//     }

//     if ( buffer->state == READSECONDCARRIAGE ){ 
//         //tengo garantizado que en c tengo el que le sigue al '\r'
//         if ( c == '\n'){
//             buffer->state = INMEDIATERETURNCARRIAGE;
//             buffer->currentPos++;
//             return '.';
//         }
//     }

//     if ( buffer->state == DOT){ 
//         if ( buffer->currentPos + 1 < buffer->len){
//             //es seguro preguntar por los 2 caracteres
//             if ( c == '\r' && buffer->auxBuffer[buffer->currentPos + 1] == '\n'){
//                 buffer->state = NORMAL;
//                 return '.';
//             }
//         } else {
//             if ( c == '\r'){
//                 buffer->state = READSECONDCARRIAGE;
//                 rebaseBuffer(buffer);
//                 buffer->currentPos++;
//                 return NEEDMOREPROCESSINFO;
//             }
//         }
//     }

//     if ( c == '\r'){
//         buffer->state = READFIRSTCARRIAGE;
//     } else if ( c == '\n'){
//         if ( buffer->state == READFIRSTCARRIAGE){
//             buffer->state = READFIRSTNEWLINE;
//         }else{
//             buffer->state = NORMAL;
//         }
//     } else if ( c == '.'){
//         if (buffer->state == READFIRSTNEWLINE){
//             buffer->state = DOT;
//         }else {
//             buffer->state = NORMAL;
//         }
//     } else {
//         buffer->state = NORMAL;
//     }

//     buffer->currentPos++;
//     return c;
// }