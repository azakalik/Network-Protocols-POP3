#define AUXBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 512
#define RECOVERERROR -1
#define GETNUMBER(n) (n - '0')
#define GREETINGMESSAGE "+OK Pop3 Server Ready\r\n"
#define WRITE_ERROR -1
#define WRITE_SUCCESS 0


#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../clients/clients.h"
#include "../logger/logger.h"
#include "popStandards.h"
#include "stdbool.h"


//---------------- LIST ----------------------------
#include "popFunctions.h"
#include "strings.h"
command_with_state validCommands[TOTALCOMMANDS] = {
    {"TOP",  emptyFunction, TRANSACTION},
    {"USER", emptyFunction, AUTHENTICATION},
    {"PASS", emptyFunction, AUTHENTICATION},
    {"STAT", emptyFunction, TRANSACTION},
    {"LIST", emptyFunction, TRANSACTION},
    {"RETR", emptyFunction, TRANSACTION},
    {"DELE", emptyFunction, TRANSACTION},
    {"NOOP", emptyFunction, TRANSACTION},
    {"RSET", emptyFunction, TRANSACTION},
    {"QUIT", emptyFunction, UPDATE}
};


//returns NULL if command not found or pointer to POP command if exists
//todo make more efficient
command_with_state * getCommand(char * command_name){
    for ( int i = 0; i < TOTALCOMMANDS; i++){
        if ( strcasecmp(validCommands[i].commandStr, command_name) == 0 ){
            return &validCommands[i];
        }
    }
    return NULL;
}


int writeToOutputBuffer(char * buffer, user_data* data ) {
    int length = strlen(buffer);
    if(getBufferFreeSpace(&data->output_buff) >= length ){
        writeDataToBuffer(&data->output_buff, buffer, length);
        return WRITE_SUCCESS;
    }
    
    return WRITE_ERROR;
}

//-------------------------LIST FUNCTIONS-----------------------------------------------------------------------

void sendGreeting(user_data * user){
    char * greetingMessage = GREETINGMESSAGE;
    writeDataToBuffer(&user->output_buff,greetingMessage,strlen(greetingMessage));
}


static int recoverSpecificMail(char * number,user_data * data, DIR * directoryPtr){
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
            if ( writeToOutputBuffer(auxBuffer, user_data) < 0 ){
                user_data->listStateData.amountSkippedFiles = currentFile;
                user_data->listStateData.state = PROCESSING;
                closedir(directoryPtr);
                return 0;
            }
            mailNumber += 1;
        }

        currentFile++;
    }

}

static int getAllMails(DIR * directoryPtr,user_data * data){
    char auxBuffer[AUXBUFFERSIZE];
    int mailNumber = 1;
    struct stat fileStat;

    struct dirent *entry;
    //-------- avanzamos el puntero hasta donde se quedo el usuario --> Reconstruir el estado
    int currentFile;
    for ( currentFile = 0; currentFile < data->listStateData.amountSkippedFiles; currentFile++ ){
        entry = readdir(directoryPtr);
        sprintf(auxBuffer,"../mails/%s/%s",data->login_info.username,entry->d_name);
        char * filePath = auxBuffer;
        if ( stat(filePath, &fileStat) < 0){
            log(ERROR,"%s", "ERROR RECOVERING STATISTICS");
            closedir(directoryPtr);
            return RECOVERERROR;
        }
        if ( S_ISREG(fileStat.st_mode) ){
            mailNumber++;
        }
    }


    //continuamos desde donde se quedo el usuario

    /*
    On  success,  readdir() returns a pointer to a dirent structure.  (This
       structure may be statically allocated; do not attempt to free(3) it.)
       If the end of the directory stream is reached, NULL is returned and er‐
       rno  is not changed.  If an error occurs, NULL is returned and errno is
       set appropriately.  To distinguish end of stream from an error, set er‐
       rno  to zero before calling readdir() and then check the value of errno
       if NULL is returned.
    */


    errno = 0;
    while ((entry = readdir(directoryPtr)) != NULL) {
        // Check if the current entry is a file
        sprintf(auxBuffer,"../mails/%s/%s",data->login_info.username,entry->d_name);
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

}

int list(char * number, char * empty, user_data * user_data){

    char auxBuffer[AUXBUFFERSIZE] = "../mails/";
    strcat(auxBuffer,user_data->login_info.username);
    
    DIR *directoryPtr;
    directoryPtr = opendir(auxBuffer);
    if (directoryPtr == NULL) {//todo improve
        return RECOVERERROR;
    }

    if ( number != NULL){
        return recoverSpecificMail(number,user_data,directoryPtr);
    } else {
        return getAllMails(user_data);
    }

    


    if(writeToOutputBuffer(".\r\n", user_data) < 0){
        user_data->listStateData.amountSkippedFiles = currentFile;
        user_data->listStateData.state = PROCESSING;
    } else {
        user_data->listStateData.amountSkippedFiles = 0;
        user_data->listStateData.state = COMPLETED;
    }
    closedir(directoryPtr);
    return 0;
}

int emptyFunction(char * arg1, char * empty, user_data * user_data){
    log(INFO, "%s", "executing empty functions");
    return 0;
}

static bool userMailDirExists(char * username){
    char auxBuffer[AUXBUFFERSIZE] = "../mails/";
    strcat(auxBuffer,username);
    return opendir(auxBuffer) != NULL; //todo error check in opendir
}

int signInWithUsername(char * username, char * empty, user_data * user_data){
    if(!userMailDirExists(username))
        return -1;

    //save username to then compare with password
    return 0;
}

int insertPassword(char * password, char * empty, user_data * user_data){
    return 0;
}

// Examples:
//              C: RETR 1
//              S: +OK 120 octets
//              S: <the POP3 server sends the entire message here>
//              S: .

//---------------- RETR ----------------------------

static void obtainFilePath(char * username, char * mailNumber, char * dest){
    sprintf(dest,"../mails/%s/%s",username,mailNumber);

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

// static int startReadingMail(user_data * data, int msgNum){
//     data->retrStateData.state = PROCESSING;
//     char auxBuffer[AUXBUFFERSIZE];
//     return 0;
// }


// static int findFileData(char * buffer, long * fileSize, int msgNum, char * userName){
//     char dirPath[AUXBUFFERSIZE];
//     char auxBuffer[AUXBUFFERSIZE];
//     sprintf(dirPath, "../mails/%s", userName);
//     DIR * directoryPtr;
//     struct dirent *entry;
//     directoryPtr = opendir(dirPath);

//     if (directoryPtr == NULL) {
//         log(ERROR,"Error opening dir: %s", dirPath);
//         return RECOVERERROR;
//     }

//     int count=1;
//     struct stat fileStat;
//     while ((entry = readdir(directoryPtr)) != NULL) {

//         sprintf(auxBuffer,"%s/%s",dirPath,entry->d_name);
//         char * filePath = auxBuffer;
//         if ( stat(filePath,&fileStat) < 0){
//             log(ERROR,"error recovering file statitistics for file %s\n",filePath);
//             closedir(directoryPtr);
//             return RECOVERERROR;
//         }
//         if ( S_ISREG(fileStat.st_mode) ){
//             if(count == msgNum){
//                 *fileSize = fileStat.st_size;
//                 strcpy(buffer, filePath);
//                 closedir(directoryPtr);
//                 return 0;
//             }
//             count++;
//         }

//     }


// }

// int retr(char * mailNo, char * empty, user_data * user_data){
//     char auxBuff[AUXBUFFERSIZE];

//     /*
//     off_t fileSize = fileStat.st_size;
//     struct stat fileStat;
//     if ( stat(auxBuff, &fileStat) < 0){
//         log(ERROR,"ERROR RECOVERING STATISTICS");
//         fclose(file);
//         return RECOVERERROR;
//     }
    
//     sprintf(auxBuff,"+OK %lld octets\r\n", (long long)fileSize);
//     */
    
//     findFileData(auxBuff,atoi(mailNo),user_data->userName);
//     obtainFilePath(user_data->login_info.username, mailNo, auxBuff);
    
    

//     FILE * file = openFile(auxBuff, user_data);

//     //ver si hay espacio en el buffer de salida y mandar la rta
//     if ( writeToOutputBuffer(auxBuff, user_data) < 0 ){
//         user_data->retrStateData.state = PROCESSING;
//         user_data->retrStateData.offset = 0; //es 0 ?
//         fclose(file);
//         return 0;
//     }
    
//     int avaiableSpace = getBufferOccupiedSpace(&user_data->output_buff);
    
// }
