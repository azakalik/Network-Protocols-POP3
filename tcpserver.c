#include "serverutils.h"
#include "util.h"
#include "logger.h"

//listenfd is the passive socket id (to listen for new requests)
int listenfd = NOT_ALLOCATED;

int main(int argc, char ** argv){

    //Ctrl + C handling
    signal(SIGINT, handle_interrupt);

	char * port;
	if (argc == 1){
		port = SERVER_PORT_STRING;
	}
	else if (argc == 2){
		port = argv[1];
	}
	else {
        log(FATAL, "Invalid number of arguments");
        exit(ERROR_CODE);
	}

    int listenfd = setupTCPPassiveSocket(port);
    
    // ===============================================================

    while (1) { // Run forever
		// Wait for a client to connect
        log(INFO, "Waiting for new connection");
		int clntSock = acceptTCPConnection(listenfd);
		if (clntSock >= 0){
			handleTCPEchoClient(clntSock);
        }
	}
    return 0;
}

void handle_interrupt(int signal){
    if (signal == SIGINT){
        puts("");
        puts("Received Ctrl+C signal.");
        puts("Closing the sockets and exiting...");

        //Close the passive socket
        if(listenfd != NOT_ALLOCATED && close(listenfd) < 0){
            log(FATAL, "Could not close passive socket in fd %d", listenfd);
            exit(ERROR_CODE);
        }

        puts("Exited succesfully.");
        exit(0);
    }
    return;
}