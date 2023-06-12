#ifndef TCPSERVERUTIL_H_
#define TCPSERVERUTIL_H_

typedef struct {
    char* port;
    int userCount;
    char **users;
} args_data;


void freeArgs(args_data * args);
args_data * parseArgs(int argc, char ** argv);

// Create, bind, and listen a new TCP server socket
int setupTCPServerSocket(const char *service);

#endif 
