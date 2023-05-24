#include "serverutils.h"
#include "util.h"

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
		err_n_die("Invalid number of arguments");
	}

    int passive_socket = setupTCPPassiveSocket(port);
    // ===============================================================

    while (1) { // Run forever
		// Wait for a client to connect
		int clntSock = acceptTCPConnection(passive_socket);
		if (clntSock < 0){
			//log
        }
		else {
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
        if(close(listenfd) < 0)
            err_n_die("Close error while attempting to close passive socket");

        puts("Exited succesfully.");
        exit(0);
    }
    return;
}