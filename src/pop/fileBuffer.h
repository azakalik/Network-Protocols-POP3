#ifndef FILEBUFFER_H
#define FILEBUFFER_H

#include <stdbool.h>
#define NOAVAILABLECONTENT -1;
#define NEEDMOREPROCESSINFO -2;
#define AUXBUFFERSIZE 512


typedef enum {
    NORMAL,
    READFIRSTCARRIAGE,
    READFIRSTNEWLINE,
    DOT,
    READSECONDCARRIAGE,
    INMEDIATERETURNCARRIAGE,
    INMEDIATERETURNNEWLINE,
} read_states;

typedef struct file_buffer{
    int len;
    int currentPos;
    char auxBuffer[AUXBUFFERSIZE];
    read_states state;
    bool readEOF;
} file_buffer;


void initializeBuffer(file_buffer * buffer);
void rebaseBuffer(file_buffer * buffer);

int readFromBuffer(file_buffer * buffer);
void setLength(file_buffer* buffer, int length);

void * getBufferPtr(file_buffer * buffer, int * SpaceAvailable);

#endif