#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <sys/socket.h> //basic socket definitions
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> //for inet_pton
#include <stdarg.h> //for variable argument functions
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

//the port the passive socket listens in
#define SERVER_PORT 8082
#define SERVER_PORT_STRING "8082"

//maximum waiting connections
#define MAX_WAITING 50

//buffer size
#define MAXLINE 4096

//to distinguish between allocated and not allocated socket ids
#define NOT_ALLOCATED -1

#define BUFSIZE 1024

//handles ctrl+c
void handle_interrupt(int signal);

//creates passive socket
int setupTCPPassiveSocket(char * port);

//blocks until a client connects
int acceptTCPConnection(int servSock);

//reads from the client and answers
int handleTCPEchoClient(int clntSocket);

#endif