#include "fileBuffer.h"
#include "../logger/logger.h"
#define NOAVAILABLECONTENT -1;
#define NEEDMOREPROCESSINFO -2;



void initializeBuffer(file_buffer * buffer){
    memset(buffer,0,sizeof(file_buffer));
}



void rebaseBuffer(file_buffer * buffer){
    if ( buffer->len < buffer->currentPos){
        log(FATAL,"ERROR IN FILE BUFFER");
    }

    int bytesToMove = buffer->len - buffer->currentPos;
    memmove(buffer->auxBuffer,buffer->auxBuffer + buffer->currentPos, bytesToMove );
    buffer->currentPos = 0;
    buffer->len = bytesToMove;
}

    
// hfoadsfadfadsfadfasdfasd\r\n.\r\n


void setEof(file_buffer * buffer){
    buffer->readEOF = true;
}

void * getBufferPtr(file_buffer * buffer, int * SpaceAvailable){
    *SpaceAvailable = AUXBUFFERSIZE - buffer->len;
    return buffer->auxBuffer + buffer->len;
}

void setLength(file_buffer* buffer, int length){
    buffer->len += length;
}
//returns amout of bytes read from buffer
int readFromBuffer(file_buffer * buffer){
    //read but with state machine, EOF was not reached
    
    if ( buffer->currentPos >= buffer->len ){
        return NOAVAILABLECONTENT;
    }

    

    int c = buffer->auxBuffer[buffer->currentPos];

    char character= (char) c;

    if ( buffer->state == INMEDIATERETURNCARRIAGE){
        buffer->state = INMEDIATERETURNNEWLINE;
        return '\r';
    }

    if ( buffer->state == INMEDIATERETURNNEWLINE){
        buffer->state = NORMAL;
        return '\n';
    }

    if ( buffer->state == READSECONDCARRIAGE ){ 
        //tengo garantizado que en c tengo el que le sigue al '\r'
        if ( c == '\n'){
            buffer->state = INMEDIATERETURNCARRIAGE;
            buffer->currentPos++;
            return '.';
        }
    }



    if ( buffer->state == DOT){ 
    
        if ( buffer->currentPos + 1 < buffer->len){
            //es seguro preguntar por los 2 caracteres
            if ( c == '\r' && buffer->auxBuffer[buffer->currentPos + 1] == '\n'){
                buffer->state = NORMAL;
                return '.';
            }
            
        } else {
            if ( c == '\r'){
                buffer->state = READSECONDCARRIAGE;
                rebaseBuffer(buffer);
                buffer->currentPos++;
                return NEEDMOREPROCESSINFO;
            }

        }
    }

    
    if ( c == '\r'){
        buffer->state = READFIRSTCARRIAGE;
    } else 
    if ( c == '\n'){
        if ( buffer->state == READFIRSTCARRIAGE){
            buffer->state = READFIRSTNEWLINE;
        }else{
            buffer->state = NORMAL;
        }
    } else 
    if ( c == '.'){
        if (buffer->state == READFIRSTNEWLINE){
            buffer->state = DOT;
        }else {
            buffer->state = NORMAL;
        }
    } else {
        buffer->state = NORMAL;
    }

    buffer->currentPos++;
    return c;


    
}
