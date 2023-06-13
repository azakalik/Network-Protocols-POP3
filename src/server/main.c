#include "../clients/clients.h"
#include "serverUtils.h"
#include <string.h>
#include "../logger/logger.h"
#include <unistd.h>
#include <signal.h>
#include "../users/users.h"

int servSock = NOT_ALLOCATED;

static bool serverRunning = true;

static void
sigterm_handler(const int signal) {
    log(INFO, "\nSignal %d, cleaning up and exiting\n",signal);
    serverRunning = false;
}
static void handleProgramTermination();

int main(int argc, char ** argv){

    //-------------------------------------USER SINGLETON INSTANCE INITIALIZATION---------------------------------------
    args_data * data = parseArgs(argc,argv);
    createSingletonInstance(data->userCount,data->users);
    freeArgs(data);



    // stdin will not be used
    close(STDIN_FILENO);
    //----------------------SOCKET CREATION---------------------------------------
    //servSock va a ser = 0 porque cerramos stdin
	char * servPort = argv[1];
	servSock = setupTCPServerSocket(servPort);
	if (servSock < 0 )
		return 1;

    handleProgramTermination();

    //-----------------------USER-DATA-INIT---------------------------------
    user_data usersData[MAX_CONNECTIONS];
    memset(usersData,0,sizeof(usersData));
    for (int i = 0; i < MAX_CONNECTIONS; i++){
        usersData[i].socket = NOT_ALLOCATED;
        usersData[i].commandState = AVAILABLE;
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