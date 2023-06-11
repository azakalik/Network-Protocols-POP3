#include "../clients/clients.h"
#include "serverUtils.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../logger/logger.h"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <signal.h>
#include "../util/util.h"
#include "../parsers/commandParser.h"

#define MAX_CONNECTIONS 500
#define NOT_ALLOCATED -1

static void handleClients(fd_set *readFd, fd_set *writeFd, user_data *usersData);
static void acceptConnection(user_data* connectedUsers,int servSock);
static void addClientsSocketsToSet(fd_set * readSet,fd_set* writeSet ,int * maxNumberFd, user_data * users);
static void handleSelectActivityError();
static void handleProgramTermination();
static void closeClient(user_data usersData[], int position);
static void closeAllClients(user_data usersData[]);

int servSock = NOT_ALLOCATED;

static bool serverRunning = true;

static void
sigterm_handler(const int signal) {
    log(INFO, "\nSignal %d, cleaning up and exiting\n",signal);
    serverRunning = false;
}

int main(int argc, char ** argv){
    // stdin will not be used
    close(STDIN_FILENO);
    //----------------------SOCKET CREATION---------------------------------------
	if (argc != 2) {
		log(FATAL, "usage: %s <Server Port>", argv[0]);
	}
	char * servPort = argv[1];
    //servSock va a ser = 0 porque cerramos stdin
	servSock = setupTCPServerSocket(servPort);
	if (servSock < 0 )
		return 1;

    handleProgramTermination();

    //-----------------------USER-DATA-INIT---------------------------------
    user_data usersData[MAX_CONNECTIONS];
    memset(usersData,0,sizeof(usersData));
    for (int i = 0; i < MAX_CONNECTIONS; i++){
        usersData[i].socket = NOT_ALLOCATED;
    }

    fd_set readFds;
    fd_set writeFds;
    int maxSock; //highest numbered socket
    while (serverRunning)
    {
        FD_ZERO(&readFds);
        FD_ZERO(&writeFds);
        FD_SET(servSock,&readFds);
        maxSock = servSock;
        //we add all sockets to sets
        addClientsSocketsToSet(&readFds,&writeFds,&maxSock,usersData);
        //we wait for select activity
        int selectStatus = select(maxSock + 1,&readFds,&writeFds,NULL, NULL);
        if (selectStatus < 0){
            handleSelectActivityError();
            continue;
        }
        
        //we check pasive socket for an incoming connection
        if ( FD_ISSET(servSock,&readFds) ){
            acceptConnection(usersData,servSock);
        }

        //we handle client`s content
        handleClients(&readFds,&writeFds,usersData);

    }
    closeAllClients(usersData);
    close(servSock);
    return 0;

}

static void closeClient(user_data usersData[], int position){
    user_data client = usersData[position];
    if(client.socket == NOT_ALLOCATED)
        return;

    log(INFO, "Closing client on fd %d and freeing its resources", client.socket);
    destroyList(client.command_list);
    close(client.socket);
    memset(&usersData[position],0,sizeof(user_data)); //to mark it as unoccupied
    usersData[position].socket = NOT_ALLOCATED;
}

static void closeAllClients(user_data usersData[]){
    for (int i = 0; i < MAX_CONNECTIONS; i++){
        closeClient(usersData, i);
    }
}

//attempts to execute the oldest command sent by a client
static void executeFirstCommand(struct command_list * list, buffer * buffer, pop_state user_status){
    if(availableCommands(list)){
        char * message;
        command_to_execute * command = getFirstCommand(list);
        if(command->callback.execute_command != NULL){
            if (user_status == command->callback.pop_state){
                command->callback.execute_command(command->arg1, command->arg2);
                message = "+OK Executing function\n";
            } else {
                message = "-ERR Invalid state\n";
            }
                
        } else {
            message = "-ERR Invalid command\n";
        }
        writeDataToBuffer(buffer, message, strlen(message));
        
        free(command);
    }
}

//todo limitar el numero de comandos a recibir
//TODO VER CUANDO SE RECIBE MAS DE UN COMANDO A LA VEZ
static void handleClients(fd_set *readFds, fd_set *writeFds, user_data *usersData)
{
    for ( int i = 0; i < MAX_CONNECTIONS ; i++){
        int clntSocket = usersData[i].socket;
        if (clntSocket == NOT_ALLOCATED)
            continue;

        if ( FD_ISSET(clntSocket,readFds) ){
            handleClientInput(&usersData[i]);
        } else if ( FD_ISSET(clntSocket,writeFds) ){
            executeFirstCommand(usersData[i].command_list, &usersData[i].output_buff, usersData[i].session_state); //fills the output buffer with the response
            writeToClient(&usersData[i]); //sends the content of output buffer to the client
        }
    }
}

static void acceptConnection(user_data* connectedUsers,int servSock){

    char addrBuffer[BUFFERSIZE];
    struct sockaddr_storage clntAddr; // Client address
	// Set length of client address structure (in-out parameter)
	socklen_t clntAddrLen = sizeof(clntAddr);

	// Wait for a client to connect
    //TODO: Preguntar a coda non-blocking
	int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);

	if (clntSock < 0) {
		log(ERROR, "%s", "accept() failed");
		exit(ACCEPT_FAILURE);
	}

    bool allocatedClient = false;
    user_data user;
    for ( int i = 0; !allocatedClient && i < MAX_CONNECTIONS; i++){
        if ( connectedUsers[i].socket == NOT_ALLOCATED ){
            connectedUsers[i].socket = clntSock;
            connectedUsers[i].session_state = AUTHENTICATION;
            connectedUsers[i].client_state = WRITING;
            connectedUsers[i].command_list = createList();
            allocatedClient = true;
            user = connectedUsers[i];
            sendGreeting(&connectedUsers[i]);
        }
    }

    if ( !allocatedClient ){
        log(ERROR,"%s", "Could not allocate client who requested to connect, users structure is full");
        close(clntSock);
        exit(EXIT_FAILURE); //todo dont exit!
    } else if (user.command_list == NULL){
        log(ERROR,"%s", "Could not allocate memory for a command list in a new connection");
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
        if (clientSocket == NOT_ALLOCATED)
            continue;

        if (users[i].client_state == READING)
            FD_SET(clientSocket,readSet);
        else if (users[i].client_state == WRITING)
            FD_SET(clientSocket,writeSet);

        if ( clientSocket > maxFd)
            maxFd = clientSocket;
    }
    *maxNumberFd = maxFd;
}


static void handleSelectActivityError(){
    switch (errno)
    {
    case EBADF:
        log(ERROR,"%s", "One or more fd in the set are not valid\n");
        break;
    case EINTR:
        log(INFO,"%s", "The select was interrupted by a signal before any request event occured\n");
        break;
    case EINVAL:
        log(ERROR,"%s", "The highest fd + 1 is negative or exeeds system limit\n");
        break;
    case ENOMEM:
        log(ERROR,"%s", "not enough memory to allocate required data for structures on select\n");
    default:
        log(ERROR,"%s", "unexpected error when handling select activity\n");
        break;
    }
}

static void handleProgramTermination(){
    // registrar sigterm es útil para terminar el programa normalmente.
    // esto ayuda mucho en herramientas como valgrind.
    // signal() esta deprecated
    struct sigaction sa;
    sa.sa_handler = sigterm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        log(FATAL, "%s", "sigaction(SIGTERM) failed");
    }

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        log(FATAL, "%s", "sigaction(SIGINT) failed");
    }
}


