#include "serverFunctions.h"
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

typedef int (*activity_handler)(user_data *);

static void handleClient(fd_set *fd, user_data *usersData, activity_handler activityCallback);
static void acceptConnection(user_data* connectedUsers,int servSock);
static void addClientsSocketsToSet(fd_set * readSet,fd_set* writeSet ,int * maxNumberFd, user_data * users);
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
    fd_set writeFds;
    int maxSock;//highest numbered socket
    
    while (FOREVER)
    {
        FD_ZERO(&readFds);
        FD_ZERO(&writeFds);
        FD_SET(servSock,&readFds);
        maxSock = servSock;
        //we add all sockets to sets
        addClientsSocketsToSet(&readFds,&writeFds,&maxSock,usersData);
        
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
        handleClient(&readFds,usersData,fetchClientInput);

        //we handle client`s write
        handleClient(&writeFds,usersData,writeToClient);

    }


    close(servSock);
    return 0;

}


static void handleClient(fd_set *fd, user_data *usersData, activity_handler activityCallback)
{
    
    for ( int i = 0; i < MAX_CONNECTIONS ; i++){
        int clientSocket = usersData[i].socket;
        if ( !FD_ISSET(clientSocket,fd) )
            continue;
        int activityStatus = activityCallback(&usersData[i]);
        if ( activityStatus < 0){
            log(FATAL,"error interacting with user\n");
        }
        log(INFO,"preforming a reading operation\n");
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
        close(clntSock);
        exit(EXIT_FAILURE);
    }
    
	// clntSock is connected to a client!
	printSocketAddress((struct sockaddr *) &clntAddr, addrBuffer);
	log(INFO, "Handling client %s", addrBuffer);

}


static void addClientsSocketsToSet(fd_set * readSet,fd_set* writeSet ,int * maxNumberFd, user_data * users){
    int maxFd = *maxNumberFd;
    for (int i = 0; i < MAX_CONNECTIONS; i++){
        int clientSocket = users[i].socket;
        if ( clientSocket > 0){
            FD_SET(clientSocket,readSet);
            FD_SET(clientSocket,writeSet);
        }
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
        log(ERROR,"unexpected error when handling select activity\n");
        break;
    }
    

}




