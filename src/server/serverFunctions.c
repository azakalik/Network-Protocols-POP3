#include "serverFunctions.h"
#include "../parsers/commandList.h"

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
        writeDataToBuffer(&client->output_buff, notSendPosition , bytesToWriteBack );
        return;
    }


    return;
}


void handleClientInput(user_data * client){
    char auxiliaryBuffer[MAXLINESIZE+1];
    int bytesRead = recv(client->socket, auxiliaryBuffer, MAXLINESIZE, 0);
    
    if ( bytesRead <= 0){
        //client closed connection, that position is released
        if ( bytesRead < 0){
            log(ERROR,"Error while doing recv for socket %d",client->socket);
        }
        log(INFO, "The client in socket %d sent a EOF. Releasing his resources...", client->socket);
        releaseSocketResources(client);
        return;
    }
    auxiliaryBuffer[bytesRead] = 0; //null terminate

    addData(client->command_list, auxiliaryBuffer);
}