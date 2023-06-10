#define AUXBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 512
#define RECOVERERROR -1
#define GETNUMBER(n) (n - '0')
#define GREETINGMESSAGE "+OK Pop3 Server Ready\r\n"
#define WRITE_ERROR -1
#define WRITE_SUCCESS 0



//---------------- LIST ----------------------------
#include "popFunctions.h"


//-------------------------LIST FUNCTIONS-----------------------------------------------------------------------

void sendGreeting(user_data * user){
    char * greetingMessage = GREETINGMESSAGE;
    writeDataToBuffer(&user->output_buff,greetingMessage,strlen(greetingMessage));
}



int getUserMails(char * username, user_data* data){

    char auxBuffer[AUXBUFFERSIZE] = "../mails/";
    strcat(auxBuffer,username);
    
    DIR *directoryPtr;
    struct dirent *entry;
    directoryPtr = opendir(auxBuffer);
    if (directoryPtr == NULL) {//todo improve
        errno = ENOENT;//error no entry
        return RECOVERERROR;
    }

    /*
    On  success,  readdir() returns a pointer to a dirent structure.  (This
       structure may be statically allocated; do not attempt to free(3) it.)

       If the end of the directory stream is reached, NULL is returned and er‐
       rno  is not changed.  If an error occurs, NULL is returned and errno is
       set appropriately.  To distinguish end of stream from an error, set er‐
       rno  to zero before calling readdir() and then check the value of errno
       if NULL is returned.
    */

    int mailNumber = 1;
    struct stat fileStat;

    //-------- avanzamos el puntero hasta donde se quedo el usuario --> Reconstruir el estado
    int currentFile;
    for ( currentFile = 0; currentFile < data->listStateData.amountSkippedFiles; currentFile++ ){
        entry = readdir(directoryPtr);
        sprintf(auxBuffer,"../mails/%s/%s",username,entry->d_name);
        char * filePath = auxBuffer;
        if ( stat(filePath, &fileStat) < 0){
            log(ERROR,"ERROR RECOVERING STATISTICS");
            closedir(directoryPtr);
            return RECOVERERROR;
        }
        if ( S_ISREG(fileStat.st_mode) ){
            mailNumber++;
        }
    }



    errno = 0;
    while ((entry = readdir(directoryPtr)) != NULL) {
        // Check if the current entry is a file
        sprintf(auxBuffer,"../mails/%s/%s",username,entry->d_name);
        char * filePath = auxBuffer;
        if ( stat(filePath,&fileStat) < 0){
            log(ERROR,"error recovering file statitistics for file %s\n",filePath);
            closedir(directoryPtr);
            return RECOVERERROR;
        }

        if ( S_ISREG(fileStat.st_mode) ){
            off_t fileSize = fileStat.st_size;
            sprintf(auxBuffer,"%d %lld\r\n",mailNumber,(long long)fileSize);
            //escribmos de forma mas elaborada al buffer de salida del usuario
            //si en el buffer de salida no hay espacio --> no escribo la data de ese mail, (tengo q iterar de vuelta hasta encontrar ese mail la proxima vez)
            if ( writeToOutputBuffer(auxBuffer, data) < 0 ){
                data->listStateData.amountSkippedFiles = currentFile;
                data->listStateData.state = PROCESSING;
                closedir(directoryPtr);
                return 0;
            }
            mailNumber += 1;
        }

        currentFile++;
    }

    if ( errno != 0 ){
        closedir(directoryPtr);
        return RECOVERERROR;
    }


    if(writeToOutputBuffer(".\r\n", data) < 0){
        data->listStateData.amountSkippedFiles = currentFile;
        data->listStateData.state = PROCESSING;
    } else {
        data->listStateData.amountSkippedFiles = 0;
        data->listStateData.state = COMPLETED;
    }
    closedir(directoryPtr);
    return 0;
}

int writeToOutputBuffer(char * buffer, user_data* data ) {
    int length = strlen(buffer);
    if(getBufferFreeSpace(&data->output_buff) >= length ){
        writeDataToBuffer(&data->output_buff, buffer, length);
        return WRITE_SUCCESS;
    }
    
    return WRITE_ERROR;
}


int emptyFunction(char * arg1, char * arg2){
    log(INFO, "executing empty functions");
    return 0;
}

// Examples:
//              C: RETR 1
//              S: +OK 120 octets
//              S: <the POP3 server sends the entire message here>
//              S: .

//---------------- RETR ----------------------------

static void obtainFilePath(char * username, char * mailNumber, char * dest){
    sprintf(dest,"../mails/%s/%d",username,mailNumber);

}


static FILE * openFile(char * path, user_data * data){
    FILE * file = fopen(path, "r");
    if(file == NULL){
        log(FATAL,"Error opening file: %s", path);
    }
    if(data->retrStateData.state == START)
        return file;
    //si el usuario ya estaba leyendo
    if ( fseek(file,data->retrStateData.offset,SEEK_SET) < 0 ){
        fclose(file);
        log(FATAL,"Error advancing to desired position in file: %s", path);
    }

    return file;
}

static int startReadingMail(user_data * data, int msgNum){
    data->retrStateData.state = PROCESSING;
    char auxBuffer[AUXBUFFERSIZE];
    
}

int retr(char * username, char * msgNum, user_data * data){
    char auxBuff[AUXBUFFERSIZE];
    obtainFilePath(username,msgNum,auxBuff);
    FILE * file = openFile(auxBuff, data);


    struct stat fileStat;
    if ( stat(auxBuff, &fileStat) < 0){
        log(ERROR,"ERROR RECOVERING STATISTICS");
        fclose(file);
        return RECOVERERROR;
    }

    if(!S_ISREG(fileStat.st_mode)){
        log(ERROR,"Error, file is not regular"); //es un error?
        fclose(file);
        return -1;
    }

    off_t fileSize = fileStat.st_size;
    sprintf(auxBuff,"+OK %lld octets\r\n", (long long)fileSize);
    //ver si hay espacio en el buffer de salida y mandar la rta
    if ( writeToOutputBuffer(auxBuff, data) < 0 ){
        data->retrStateData.state = PROCESSING;
        data->retrStateData.offset = 0; //es 0 ?
        fclose(file);
        return 0;
    }
    
    int avaiableSpace = getBufferOccupiedSpace(&data->output_buff);


    size_t bytesRead = fread()

    //leemos del archivo
    
}
