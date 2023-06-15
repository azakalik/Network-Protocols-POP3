#ifndef CLIENTS_H
#define CLIENTS_H
#include "../pop/popStandards.h"
#include "../buffer/circularBuffer.h"
#include "../mailsCache/mailsCache.h"
#include "../pop/fileBuffer.h"
#define ACCEPT_FAILURE -1

typedef enum {
    AVAILABLE, //can execute new command
    PROCESSING, //the current command hasn't finished yet
} command_execute_state;

typedef struct {
    long offset;
    file_buffer fileReadingBuffer;
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
    char username[MAXARGSIZE+1];
    login_status login_status;
} login_info;

typedef struct {
    struct command_list * command_list;
    buffer output_buff;
    pop_state session_state;
    client_state client_state;
    int socket;
    retr_state_data retrStateData;
    login_info login_info;
    void *currentCommand; //command currently executing
    command_execute_state commandState; //command execution status (tells if you can execute a new command)
    mailCache * mailCache;
} user_data;



typedef struct {
    char * username;
    char * password;
} registered_users;

void writeToClient(user_data * client);
void readFromClient(user_data * client);
void initClient(user_data * client, int sockNum);
void closeClient(user_data * usersData);
int setupUDPServerSocket(char * service);

#endif