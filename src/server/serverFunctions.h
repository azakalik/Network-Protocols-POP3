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

#define TEMPBUFFER 512
#define MAXCOMMANDSIZE 4 //without null term
#define MAXARGSIZE 40 //without null term
#define MAXLINESIZE 255 //with final crlf
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


typedef struct {
    struct command_list * command_list;
    user_buffer output_buff;
    pop_state session_state;
    client_state client_state;
    int socket;
    user_buffer fileBuffer;
} user_data;

void writeToClient(user_data * client);
void handleClientInput(user_data * client);

#define ACCEPT_FAILURE -1

#endif