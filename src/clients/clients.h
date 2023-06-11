#ifndef CLIENTS_H
#define CLIENTS_H
#include "../pop/popStandards.h"
#include "../buffer/circularBuffer.h"
#define ACCEPT_FAILURE -1

typedef enum {
    AVAILABLE, //can execute new command
    PROCESSING, //the current command hasn't finished yet
} command_execute_state;

typedef struct {
    int amountSkippedFiles;
    int requestedMail;
} list_state_data;

typedef struct {
    long offset;
} retr_state_data;

typedef enum {
    READING, //the server is reading from the client
    WRITING, //the server is writing to the client
} client_state;

typedef enum {
    ANONYMOUS, //user hasn't inserted his username yet
    KNOWN_USERNAME, //user inserted his username but not the password yet (or an invalid password)
    LOGGED_IN, //user has inserted correct username and password combination
} login_status;

typedef struct {
    char username[MAXARGSIZE];
    login_status login_status;
} login_info;


typedef struct {
    struct command_list * command_list;
    buffer output_buff;
    pop_state session_state;
    client_state client_state;
    int socket;
    long fileOffset;
    buffer fileReadingBuffer;
    list_state_data listStateData;
    retr_state_data retrStateData;
    login_info login_info;
    char userName[MAXARGSIZE+1];

    //aca va comando a ejecutar (el ptr con la data);
    //aca va el status --> AVAILABLE o PROCESSING -->    
    void *currentCommand;
    command_execute_state commandState;
} user_data;

void writeToClient(user_data * client);
void handleClientInput(user_data * client);

#endif