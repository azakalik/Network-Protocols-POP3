
#define BUFFERSIZE 2048

typedef struct {
    char buffer[BUFFERSIZE];
    int count;
    int readPtr;
    int writePtr;
} user_buffer;


int getBufferFreeSpace(user_buffer * buff);

int isBufferFull(user_buffer * buff);

int isBufferEmpty(user_buffer * buff);

void writeDataToBuffer(user_buffer * buff, char * src, int len);


int readDataFromBuffer(user_buffer * buff, char * dest, int len);
