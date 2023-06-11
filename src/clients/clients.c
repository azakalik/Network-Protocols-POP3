#include "../parsers/commandParser.h"
#include "../logger/logger.h"
#include "clients.h"
#include <unistd.h> // close()
#include <string.h> // memset()
#include <sys/types.h> // send()
#include <sys/socket.h> // send()
#include <errno.h>

#define NOT_ALLOCATED -1 //todo esto mismo esta declarado en main, buscar una solucion

void releaseSocketResources(user_data * data){
    close(data->socket);
    destroyList(data->command_list);
    memset(data,0,sizeof(user_data));
    data->socket = NOT_ALLOCATED;
}

void writeToClient(user_data * client){
    int toWrite = getBufferOccupiedSpace(&client->output_buff);
    if(toWrite!=0){
        char auxiliaryBuffer[toWrite];
        
        readDataFromBuffer(&client->output_buff, auxiliaryBuffer, toWrite);
        int bytesSent = send(client->socket, auxiliaryBuffer, toWrite, 0); //TODO: add flags
        if ( bytesSent < 0 ){
            log(ERROR,"Could not send data to buffer %d",client->socket);
            releaseSocketResources(client);
            return;
        } 
        
        if (bytesSent < toWrite){
            int bytesToWriteBack = toWrite - bytesSent;
            char * notSendPosition = auxiliaryBuffer + bytesSent; 
            writeDataToBuffer(&client->output_buff, notSendPosition , bytesToWriteBack );
        }
    }

    if(isBufferEmpty(&client->output_buff) && !availableCommands(client->command_list))
        client->client_state = READING;

    return;
}


void handleClientInput(user_data * client){
    char auxiliaryBuffer[MAXCOMMANDLENGTH+1];
    int bytesRead = recv(client->socket, auxiliaryBuffer, MAXCOMMANDLENGTH, 0);
    
    if ( bytesRead <= 0){
        //client closed connection, that position is released
        if ( bytesRead < 0){
            log(ERROR,"Error while doing recv for socket %d. Errno %d",client->socket, errno);
            return;
        }
        log(INFO, "The client in socket %d sent a EOF. Releasing his resources...", client->socket);
        releaseSocketResources(client);
        return;
    }
    auxiliaryBuffer[bytesRead] = 0; //null terminate

    addData(client->command_list, auxiliaryBuffer);
    client->client_state = WRITING;
}