#ifndef TCPSERVERUTIL_H_
#define TCPSERVERUTIL_H_

#include <stdio.h>
#include <sys/socket.h>


// Create, bind, and listen a new TCP server socket
int setupTCPServerSocket(const char *service);


#endif 
