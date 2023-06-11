#ifndef CIRCULAR_BUFFER
#define CIRCULAR_BUFFER

#define BUFFERSIZE 2048
typedef struct { //todo esto deberia estar en el .c
    char buffer[BUFFERSIZE];
    int count;
    int readPtr;
    int writePtr;
} buffer;

int getBufferOccupiedSpace(buffer * buff);

int getBufferFreeSpace(buffer * buff);

int isBufferFull(buffer * buff);

int isBufferEmpty(buffer * buff);

void writeDataToBuffer(buffer * buff, char * src, int len);

int readDataFromBuffer(buffer * buff, char * dest, int len);

#endif