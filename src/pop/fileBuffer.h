#ifndef FILEBUFFER_H
#define FILEBUFFER_H

#include <stdbool.h>
#define NOAVAILABLECONTENT -1;
#define NEEDMOREPROCESSINFO -2;

typedef struct file_buffer file_buffer;

void initializeBuffer(file_buffer * buffer);
void rebaseBuffer(file_buffer * buffer);

int readFromBuffer(file_buffer * buffer);
void setLength(file_buffer* buffer, int length);

void * getBufferPtr(file_buffer * buffer, int * SpaceAvailable);

#endif