
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

typedef enum {
    GREETING,
    AUTHENTICATION,
    TRANSACTION,
    UPDATE,
} server_state;




typedef struct {    
    user_buffer entry_buff;
    user_buffer output_buff;
    server_state session_state;
    int socket;
} user_data;

int fetchClientInput(user_data * client);

int writeToClient(user_data * client);

int handleClientGreeting(user_data * client);

#define ACCEPT_FAILURE -1