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