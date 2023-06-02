#include "serverFunctions.h"

void releaseSocketResources(user_data * data){
    close(data->socket);
    memset(data,0,sizeof(user_data));
}

void writeToClient(user_data * client){
    int toWrite = getBufferOccupiedSpace(&client->output_buff);
    if(toWrite==0)
        return;
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
        writeDataToBuffer(&client->entry_buff, notSendPosition , bytesToWriteBack );
        return;
    }


    return;
}


void handleClientInput(user_data * client){
    int maxPopSize = 1024; //todo !!!
    char auxiliaryBuffer[maxPopSize];
    int bytesRead = recv(client->socket, auxiliaryBuffer, maxPopSize, 0);
    
    if ( bytesRead <= 0){
        //client closed connection, that position is released
        if ( bytesRead < 0){
            log(ERROR,"Error while doing recv for socket %d",client->socket);
        }
        log(INFO, "The client in socket %d sent a EOF. Releasing his resources...", client->socket);
        releaseSocketResources(client);
        return;
    }

    //todo populate command array
}