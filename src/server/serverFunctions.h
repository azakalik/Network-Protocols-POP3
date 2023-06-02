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

// struct fullCommand {
//     COMMAND,
//     ARG1,
//     ARG2,
//     struct status commandStatus,
// };

// struct status {
//     WRITINGCOMMAND,
//     WRITINGARG1,
//     WRITINGARG2,
//     COMPLETE,
// };

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
    user_buffer entry_buff;
    user_buffer output_buff;
    pop_state session_state;
    client_state client_state;
    int socket;
} user_data;

void writeToClient(user_data * client);
void handleClientInput(user_data * client);

#define ACCEPT_FAILURE -1

#endif