#include "serverutils.h"
#include "util.h"

int setupTCPPassiveSocket(char * port){
    // Construct the server address structure
	struct addrinfo hint;                   // Criteria for address match
	memset(&hint, 0, sizeof(hint));         // Zero out structure
	hint.ai_family = AF_UNSPEC;             // Any address family
	hint.ai_flags = AI_PASSIVE;             // Accept on any address/port
	hint.ai_socktype = SOCK_STREAM;         // Only stream sockets
	hint.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

	//port == NULL ? port = SERVER_PORT_STRING : itoa(port)	TODO

	struct addrinfo * socket_list; 			// List of server addresses
    if(getaddrinfo(NULL, port, &hint, &socket_list)  != 0 ){
        err_n_die("getaddrinfo failed");
	}

	int servSock = -1;
	// Iterate the list and bind to the first available socket
	for (struct addrinfo *addr = socket_list; addr != NULL && servSock == -1; addr = addr->ai_next) {
		errno = 0;
		// Create a TCP socket
        if((servSock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol)) < 0){
            //todo log
			continue;       // Socket creation failed; try next address
		}

		// Bind to ALL the address and set socket to listen
		if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) && (listen(servSock, MAX_WAITING) == 0)) {
			//log
		} else {
            //log
			close(servSock);  // Close and try with the next one
			servSock = -1;
		}
	}

	freeaddrinfo(socket_list);

	return servSock;
}

int acceptTCPConnection(int servSock) {
	struct sockaddr_storage clntAddr; // Client address
	// Set length of client address structure (in-out parameter)
	socklen_t clntAddrLen = sizeof(clntAddr);

	// Wait for a client to connect
	int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
	if (clntSock < 0) {
		//log
		return -1;
	}

	// clntSock is connected to a client!
    //log
	return clntSock;
}

int handleTCPEchoClient(int clntSocket) {
	char buffer[BUFSIZE]; // Buffer for echo string
	// Receive message from client
	ssize_t numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
	if (numBytesRcvd < 0) {
		return -1;   // TODO definir codigos de error
	}

	// Send received string and receive again until end of stream
	while (numBytesRcvd > 0) { // 0 indicates end of stream
		// Echo message back to client
		ssize_t numBytesSent = send(clntSocket, buffer, numBytesRcvd, 0);
		if (numBytesSent < 0) {
			return -1;   // TODO definir codigos de error
		}
		else if (numBytesSent != numBytesRcvd) {
			return -1;   // TODO definir codigos de error
		}

		// See if there is more data to receive
		numBytesRcvd = recv(clntSocket, buffer, BUFSIZE, 0);
		if (numBytesRcvd < 0) {
			return -1;   // TODO definir codigos de error
		}
	}

	close(clntSocket);
	return 0;
}