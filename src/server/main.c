#include "serverData.h"
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
#define FOREVER 1
#define MAX_CONNECTIONS 500


static void handleClientRead(fd_set * fd, user_data * usersData);
static void acceptConnection(user_data* connectedUsers,int servSock);
static void addClientsSocketsToSet(fd_set * set, int * maxNumberFd, user_data * users);
static void handleSelectActivityError();

int main(int argc, char ** argv){

    //----------------------SOCKET CREATION---------------------------------------
	if (argc != 2) {
		log(FATAL, "usage: %s <Server Port>", argv[0]);
	}
	char * servPort = argv[1];
	int servSock = setupTCPServerSocket(servPort);
	if (servSock < 0 )
		return 1;

    //-----------------------USER-DATA-INIT---------------------------------
    user_data usersData[MAX_CONNECTIONS];
    memset(usersData,0,sizeof(usersData));


    fd_set readFds;
    int maxSock;//highest numbered socket
    while (FOREVER)
    {
        FD_ZERO(&readFds);
        FD_SET(servSock,&readFds);
        maxSock = servSock;
        addClientsSocketsToSet(&readFds,&maxSock,usersData);

        //we wait for select activity
        int selectStatus = select(maxSock + 1,&readFds,NULL,NULL,NULL);
        if (selectStatus < 0){
            handleSelectActivityError();
            //TODO: preguntar a coda como manejar errores
            continue;
        }

        //we check master socket for an incoming connection
        if ( FD_ISSET(servSock,&readFds)){
            acceptConnection(usersData,servSock);
        }

        //we handle client`s content
        handleClientRead(&readFds,usersData);
        

    }

}


static void handleClientRead(fd_set * fd, user_data * usersData){

    char buffer[BUFFERSIZE];//Just for client connection accept testing
    int valRead;
    
    for ( int i = 0; i < MAX_CONNECTIONS ; i++){
        int clientSocket = usersData[i].socket;
        if ( !FD_ISSET(clientSocket,fd) )
            continue;
        valRead = read(clientSocket,buffer,BUFFERSIZE);
        if (  valRead == 0){
            log(INFO,"Client decided to close connection");
        } else {
            log(INFO,"Continue with the connection");
        }
        
    }
}

static void acceptConnection(user_data* connectedUsers,int servSock){

    char addrBuffer[BUFFERSIZE];
    struct sockaddr_storage clntAddr; // Client address
	// Set length of client address structure (in-out parameter)
	socklen_t clntAddrLen = sizeof(clntAddr);

	// Wait for a client to connect
	int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
	if (clntSock < 0) {
		log(ERROR, "accept() failed");
		exit(ACCEPT_FAILURE);
	}

    bool allocatedClient = false;
    for ( int i = 0; !allocatedClient && i < MAX_CONNECTIONS ;i++){
        if ( connectedUsers[i].socket == 0 ){
            connectedUsers[i].socket = clntSock;
            connectedUsers[i].session_state = GREETING;
            allocatedClient = true;
        }
    }

    if ( !allocatedClient ){
        log(ERROR,"Could not allocate client who requested to connect, users structure is full\n");
        exit(EXIT_FAILURE);
    }
    
	// clntSock is connected to a client!
	printSocketAddress((struct sockaddr *) &clntAddr, addrBuffer);
	log(INFO, "Handling client %s", addrBuffer);

}


static void addClientsSocketsToSet(fd_set * set, int * maxNumberFd, user_data * users){
    int maxFd = *maxNumberFd;
    for (int i = 0; i < MAX_CONNECTIONS; i++){
        int clientSocket = users[i].socket;
        if ( clientSocket > 0)
            FD_SET(clientSocket,set);
        if ( clientSocket > maxFd)
            maxFd = clientSocket;
    }
    *maxNumberFd = maxFd;
}


static void handleSelectActivityError(){
    switch (errno)
    {
    case EBADF:
        log(ERROR,"One or more fd in the set are not valid\n");
        break;
    case EINTR:
        log(ERROR,"The select was interrupted by a signal before any request event occured\n");
        break;
    case EINVAL:
        log(ERROR,"The highest fd + 1 is negative or exeeds system limit\n");
        break;
    case ENOMEM:
        log(ERROR,"not enough memory to allocate required data for structures on select\n")
    default:
        break;
    }
    

}




