
#include "circularBuffer.h"
#include "../logger/logger.h"
#include <stdbool.h>

int getBufferFreeSpace(user_buffer *buff)
{
    return BUFFERSIZE - buff->count;
}

int getBufferOccupiedSpace(user_buffer *buff)
{
    return buff->count;
}

int isBufferFull(user_buffer *buff)
{
    return buff->count == BUFFERSIZE;
}

int isBufferEmpty(user_buffer *buff)
{
    return buff->count == 0;
}

void writeDataToBuffer(user_buffer *buff, char *src, int len)
{
    for (int i = 0; i < len; i++)
    {
        if (isBufferFull(buff))
        {
            log(FATAL, "Buffer is full, this shouldnt happen");
        }
        buff->buffer[buff->writePtr] = src[i];
        buff->writePtr = (buff->writePtr + 1) % BUFFERSIZE;
        buff->count++;
    }
}

int readDataFromBuffer(user_buffer *buff, char *dest, int len)
{
    int bytesRead = 0;

    bool readingData = true;
    for (int i = 0; readingData && i < len; i++)
    {
        if (isBufferEmpty(buff))
        {
            readingData = false;
            continue;
        }

        dest[i] = buff->buffer[buff->readPtr];
        buff->readPtr = (buff->readPtr + 1) % BUFFERSIZE;
        buff->count--;
        bytesRead++;
    }

    return bytesRead;
}


// USER

// RETR 1\r\nLIST\r\nRE
// vamos a buffer auxiliar --> RETR 1\r\nLIST\r\nRE


// LINKED LIST --> struct commandList

typedef enum {
    WRITINGCOMMAND,
    WRITINGARG1,
    WRITINGARG2,
    COMPLETE
} command_state;

typedef struct  {
    char * comamand;
    char * arg1;
    char * arg2;
    command_state state;
} completecommand;