#ifndef SERVER_FUNCTIONS
#define SERVER_FUNCTIONS

#include <stdbool.h>
#include "serverUtils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../logger/logger.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include "../util/util.h"
#include "circularBuffer.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define TEMPBUFFER 512
#define MAXCOMMANDSIZE 4 //without null term
#define MAXARGSIZE 40 //without null term
#define MAXLINESIZE 2 //with final crlf
//max size for the first line of server response (including crlf)
#define MAXANSWERSIZE 512

typedef enum {
    READING,
    WRITING,
} client_state;

typedef enum {
    AUTHENTICATION,
    TRANSACTION,
    UPDATE,
} pop_state;





typedef enum {
    START,
    COMPLETED,
    PROCESSING,
} processing_state;

typedef struct {
    int amountSkippedFiles;
    processing_state state;
} list_state_data;

typedef struct {
    long offset;
    processing_state state;
} retr_state_data;


typedef struct {
    struct command_list * command_list;
    user_buffer output_buff;
    pop_state session_state;
    client_state client_state;
    int socket;
    long fileOffset;
    user_buffer fileReadingBuffer;
    list_state_data listStateData;
    retr_state_data retrStateData;
    char userName[TEMPBUFFER];
} user_data;

void writeToClient(user_data * client);
void handleClientInput(user_data * client);

#define ACCEPT_FAILURE -1

#endif