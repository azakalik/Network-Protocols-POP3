#include "serverFunctions.h"

void releaseSocketResources(user_data * data){
    close(data->socket);
    memset(data,0,sizeof(user_data));
}

void fetchClientInput(user_data * client){
    if(isBufferFull(&client->entry_buff)){
        return;
    }
    
    int freeSpace = getBufferFreeSpace(&client->entry_buff);
    char auxiliaryBuffer[freeSpace];
    int bytesRead = recv(client->socket, auxiliaryBuffer, freeSpace, 0);
    
    if ( bytesRead <= 0){
        //client closed connection, that position is released
        if ( bytesRead < 0){
            log(ERROR,"Error while doing recv for socket %d",client->socket);
        }
        releaseSocketResources(client);
        return;
    }


    writeDataToBuffer(&client->entry_buff,auxiliaryBuffer, bytesRead);
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
    }

    return;
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