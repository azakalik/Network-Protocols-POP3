#include "serverFunctions.h"



int fetchClientInput(user_data * client){
    //we can read from client socket, operation is non blocking
    switch (client->session_state)
    {
    case GREETING:
        break;//greeting is a write operation
    
    default:
        break;
    }
    return 0;
}


int writeToClient(user_data * client){
    switch (client->session_state)
    {
    case GREETING:
        handleClientGreeting(client);
        break;
    
    default:
        break;
    }
    return 0;
}

int handleClientGreeting(user_data * client){
    //TODO: charlar con zaka un poco de como seria si el cliente no lee
    char * greeting = "+OK POP3 server ready";
    int bytesSent = send(client->socket,greeting,strlen(greeting),0);
    if ( bytesSent < 0){
        log(FATAL,"could not send bytes to client for greeting");
    }
    return bytesSent;
}