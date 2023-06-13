#include "../clients/clients.h"
#include "serverUtils.h"
#include "../logger/logger.h"
#include <errno.h>
#include <sys/types.h> // getaddrinfo()
#include <sys/socket.h> // getaddrinfo()
#include <netdb.h> // getaddrinfo()
#include <string.h> // memset()
#include "../util/util.h" // printSocketAddress()
#include <unistd.h> // close()
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../users/users.h"
#include "../pop/popFunctions.h"
#include "../stats/stats.h"
#include <signal.h>
#include "../parsers/commandParser.h"


#define MAXPENDING 5 // Maximum outstanding connection requests
#define BUFSIZE 256
#define MAX_ADDR_BUFFER 128



static char addrBuffer[MAX_ADDR_BUFFER];
/*
 ** Se encarga de resolver el número de puerto para service (puede ser un string con el numero o el nombre del servicio)
 ** y crear el socket pasivo, para que escuche en cualquier IP, ya sea v4 o v6
 */
int setupTCPServerSocket(const char *service) {
	// Construct the server address structure
	struct addrinfo addrCriteria;                   // Criteria for address match
	memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
	addrCriteria.ai_family = AF_INET6;             // Any address family
	addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
	addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
	addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

	struct addrinfo *servAddr; 			// List of server addresses
	int rtnVal = getaddrinfo(NULL, service, &addrCriteria, &servAddr);
	if (rtnVal != 0) {
		log(FATAL, "getaddrinfo() failed %s", gai_strerror(rtnVal));
		return -1;
	}

	///////////////////////////////////////////////////////////// IPv6 
	// En realiad no es necesario crear dos sockets, lo hacemos como ejercicio, podemos setear las opciones
	// para que IPv6 acepte ambos (dual stack socket)
	

	int servSock = -1;
	// Intentamos ponernos a escuchar en alguno de los puertos asociados al servicio, sin especificar una IP en particular
	// Iteramos y hacemos el bind por alguna de ellas, la primera que funcione, ya sea la general para IPv4 (0.0.0.0) o IPv6 (::/0) .
	// Con esta implementación estaremos escuchando o bien en IPv4 o en IPv6, pero no en ambas
	for (struct addrinfo *addr = servAddr; addr != NULL && servSock == -1; addr = addr->ai_next) {
		errno = 0;
		// Create a TCP socket
		servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
		if (servSock < 0) {
			log(DEBUG, "%s", "Cant't create socket on given address ");  
			continue;       // Socket creation failed; try next address
		}


		///////////////////////////////////////////////////////////// IPv6 
		// En realiad no es necesario crear dos sockets, lo hacemos como ejercicio, podemos setear las opciones
		// para que IPv6 acepte ambos (dual stack socket)
		
		// Enable the reuse of local address and port
		if(setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) == -1){
			log(ERROR, "%s", "Error setting socket option: SO_REUSEADDR");
			exit(1);
		}
		
		int on = 0;
		if ( setsockopt(servSock, IPPROTO_IPV6, IPV6_V6ONLY, (const void *)&on, sizeof(on)) < 0 ){
			log(ERROR,"%s", "Cant`t set socket option to recieve ipv4 and ipv6 connections")
			exit(1);
		}
		

		// Bind to ALL the address and set socket to listen
		if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) && (listen(servSock, MAXPENDING) == 0)) {
			// Print local address of socket
			struct sockaddr_storage localAddr;
			socklen_t addrSize = sizeof(localAddr);
			if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) >= 0) {
				printSocketAddress((struct sockaddr *) &localAddr, addrBuffer);
				log(INFO, "Binding to %s", addrBuffer);
			}
		} else {
			log(DEBUG, "Cant't bind %s", strerror(errno));  
			close(servSock);  // Close and try with the next one
			servSock = -1;
		}
	}

	freeaddrinfo(servAddr);

	return servSock;
}

void acceptConnection(user_data* connectedUsers,int servSock){

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
            allocatedClient = initClient(&connectedUsers[i], clntSock);
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

    //client was successfully allocated, we add it to the monitoring protocol statistics structure
    addConcurrentConnectionToStats();

    
	// clntSock is connected to a client!
	printSocketAddress((struct sockaddr *) &clntAddr, addrBuffer);
	log(INFO, "Handling client %s", addrBuffer);

}


static char * strduplicate(char * str){
	int len = strlen(str) + 1;
	char * newStr = malloc(len);
	if ( newStr == NULL){
		log(FATAL,"%s","NO MEMORY");
	}
	strcpy(newStr,str);
	return newStr;
}



args_data * parseArgs(int argc, char ** argv){
	args_data * args = calloc(1,sizeof(args_data));

	if ( argc <= 2){
		log(ERROR,"%s","Invalid arguments");
		exit(1);
	}
	
	bool error = false;
	for ( int i = 0; i < argc && !error; i++){
		if ( i == 0){
			//skip program name
			continue;
		}

		if ( i == 1){
			int port = atoi(argv[i]);
			if ( port < 1024){
				log(ERROR,"%s","Invalid Usage: <PORT>, WHERE PORT >1024\n");
				exit(1);
			}
			continue;
		}

		if(strcmp(argv[i],"-u") == 0){
			if ( i + 1 < argc){
				args->userCount++;
				args->users = realloc(args->users, args->userCount * sizeof(char *));
				if ( args->users == NULL){
					log(FATAL,"%s","NO MEMORY");
				}
				args->users[args->userCount - 1] = strduplicate(argv[i + 1]);
				i++;
			} else {
				log(ERROR,"%s","Invalid Usage: format -u must be followed by user:pass\n");
				error = true;
			}
		} 
	}

	if ( error || args->userCount > MAXUSERS){
		freeArgs(args);
		exit(1);
	}

	return args;

}


void freeArgs(args_data * args){
	for ( int i = 0; i < args->userCount; i++){
		free(args->users[i]);
	}
	free(args->users);
	free(args);
}

void closeAllClients(user_data usersData[]){
    for (int i = 0; i < MAX_CONNECTIONS; i++){
        closeClient(&usersData[i]);
    }
}

//attempts to execute the oldest command sent by a client
void executeFirstCommand(struct command_list * list, user_data * user_data){
    if(availableCommands(list)){
        char * message;

        if(user_data->commandState == AVAILABLE){
            command_to_execute * command = getFirstCommand(list);
        
            //TODO: Escritura no controlada a buffer del cliente en caso de que este lleno
            if(command->callback.execute_command == NULL){
                message = "-ERR Invalid command\n";
                writeDataToBuffer(&user_data->output_buff, message, strlen(message));
                free(command);
                return;
            } else if (user_data->session_state != command->callback.pop_state_needed){
                message = "-ERR Invalid state\n";
                writeDataToBuffer(&user_data->output_buff, message, strlen(message));
                free(command);
                return;
            }
            user_data->currentCommand = (void *)command;
        }
        command_to_execute * command = (command_to_execute *)user_data->currentCommand;
        int functionStatus = command->callback.execute_command(command->arg1, command->arg2, user_data);
        if (functionStatus == COMMANDCOMPLETED){
            user_data->commandState = AVAILABLE;
            free(user_data->currentCommand);
        } else if (functionStatus == INCOMPLETECOMMAND) {
            user_data->commandState = PROCESSING;
        } else {
            log(ERROR,"%s","An error occured while executing a command")
            //entonces ocurrio un error al ejecutar el comando: TODO Pensar como manejar este error
        }
    }
}

void handleClients(fd_set *readFds, fd_set *writeFds, user_data *usersData)
{
    for ( int i = 0; i < MAX_CONNECTIONS ; i++){
        int clntSocket = usersData[i].socket;
        if (clntSocket == NOT_ALLOCATED)
            continue;

        if ( FD_ISSET(clntSocket,readFds) ){
            readFromClient(&usersData[i]);
        } else if ( FD_ISSET(clntSocket,writeFds) ){
            executeFirstCommand(usersData[i].command_list, &usersData[i]); //fills the output buffer with the response
            writeToClient(&usersData[i]); //sends the content of output buffer to the client
            //if user->popState == update => quit
        }
    }
}




void addClientsSocketsToSet(fd_set * readSet,fd_set* writeSet ,int * maxNumberFd, user_data * users){
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


void handleSelectActivityError(){
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

