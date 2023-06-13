#define AUXBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 512
#define RECOVERERROR -1
#define GREETINGMESSAGE "+OK Pop3 Server Ready\r\n"
#define WRITE_ERROR -1
#define WRITE_SUCCESS 0


#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../logger/logger.h"
#include "popStandards.h"
#include "stdbool.h"
#include "../users/users.h"
#include "popFunctions.h"

//----------------FUNCTION-PROTOTYPES--------------------------
executionStatus checkValidUsername(char * username, char * empty, user_data * data);
executionStatus checkValidPassword(char * password, char * empty, user_data * data);
executionStatus emptyFunction(char * arg1, char * empty, user_data * user_data);
executionStatus quit(char *, char *, user_data * user_data);
executionStatus noop(char *, char *, user_data * user_data);
executionStatus dele(char * toDelete, char *, user_data * user_data);
executionStatus rset(char *, char *, user_data * user_data);


//---------------- LIST-OF-COMMANDS----------------------------
#include "popFunctions.h"
#include "strings.h"
command_with_state validCommands[TOTALCOMMANDS] = {
    {"TOP",  emptyFunction,         TRANSACTION},
    {"USER", checkValidUsername,    AUTHENTICATION},
    {"PASS", checkValidPassword,    AUTHENTICATION},
    {"STAT", emptyFunction,         TRANSACTION},
    {"LIST", emptyFunction,         TRANSACTION},
    {"RETR", emptyFunction,         TRANSACTION},
    {"DELE", dele,                  TRANSACTION},
    {"NOOP", noop,                  TRANSACTION},
    {"RSET", rset,                  TRANSACTION},
    {"QUIT", quit,                  ANY},
    {"CAPA", emptyFunction,         AUTHENTICATION},
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

//todo improve!!!
int writeToOutputBuffer(char * buffer, user_data* data ) { //todo use it in every pop function
    int length = strlen(buffer);
    if(getBufferFreeSpace(&data->output_buff) >= length ){
        writeDataToBuffer(&data->output_buff, buffer, length);
        return WRITE_SUCCESS;
    }
    
    return WRITE_ERROR;
}





//------------------------USER FUNCTION------------------------------------------------------------------------

executionStatus checkValidUsername(char * username, char * empty, user_data * data){
    char * message;
    if ( validUsername(username) ){
        message = "+OK User accepted\r\n";
    } else {
        message = "-ERR Invalid username\r\n";
    }
    int len = strlen(message);
    if ( getBufferFreeSpace(&data->output_buff) >= len){
        writeDataToBuffer(&data->output_buff,message,len);
        strcpy(data->login_info.username,username);
        return COMMANDCOMPLETED;
    } 
    return INCOMPLETECOMMAND;
}


executionStatus checkValidPassword(char * password, char * empty, user_data * data){
    char * message;
    if ( validPassword(data->login_info.username,password) ){
        message = "+OK Welcome\r\n";
    } else {
        message = "-ERR Authentication Failed\r\n";
    }
    int len = strlen(message);
    if ( getBufferFreeSpace(&data->output_buff) >= len){
        writeDataToBuffer(&data->output_buff,message,len);
        data->session_state = TRANSACTION;
        return COMMANDCOMPLETED;
    }
    return INCOMPLETECOMMAND;
}

//-------------------------LIST FUNCTIONS-----------------------------------------------------------------------

int sendGreeting(user_data * user){
    char * greetingMessage = GREETINGMESSAGE;
    writeDataToBuffer(&user->output_buff,greetingMessage,strlen(greetingMessage));
    return COMMANDCOMPLETED;
}


static int recoverSpecificMail(char * userMailNumber,user_data * data, DIR * directoryPtr){
    char auxBuffer[AUXBUFFERSIZE];
    struct stat fileStat;
    struct dirent *entry;
    errno = 0;
    int mailNumber = 1;
    int userMailNum = atoi(userMailNumber);

    while ((entry = readdir(directoryPtr)) != NULL) {
        // Check if the current entry is a file
        sprintf(auxBuffer,"../mails/%s/%s",data->login_info.username,entry->d_name);
        char * filePath = auxBuffer;
        if ( stat(filePath,&fileStat) < 0){
            log(ERROR,"error recovering file statitistics for file %s\r\n",filePath);
            closedir(directoryPtr);
            return COMMANDERROR;
        }

        if ( !S_ISREG(fileStat.st_mode) ){
            continue;
        }

        if ( mailNumber == userMailNum ){
            off_t fileSize = fileStat.st_size;
            sprintf(auxBuffer,"%d %lld\r\n",mailNumber,(long long)fileSize);
            if ( writeToOutputBuffer(auxBuffer,data) < 0){
                data->listStateData.requestedMail = mailNumber;
                return INCOMPLETECOMMAND;
            } 
            return COMMANDCOMPLETED;
        }
        mailNumber += 1;
    }

    return COMMANDERROR;

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
            return COMMANDERROR;
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
            return COMMANDERROR;
        }

        if ( S_ISREG(fileStat.st_mode) ){
            off_t fileSize = fileStat.st_size;
            sprintf(auxBuffer,"+ OK %d %lld\r\n",mailNumber,(long long)fileSize);
            //escribmos de forma mas elaborada al buffer de salida del usuario
            //si en el buffer de salida no hay espacio --> no escribo la data de ese mail, (tengo q iterar de vuelta hasta encontrar ese mail la proxima vez)
            if ( writeToOutputBuffer(auxBuffer, data) < 0 ){
                data->listStateData.amountSkippedFiles = currentFile;
                closedir(directoryPtr);
                return INCOMPLETECOMMAND;
            }
            mailNumber += 1;
        }

        currentFile++;
    }

    if(writeToOutputBuffer(".\r\n", data) < 0){
        data->listStateData.amountSkippedFiles = currentFile;
        return INCOMPLETECOMMAND;
    } else {
        data->listStateData.amountSkippedFiles = 0;
        return COMMANDCOMPLETED;
    }

}

int list(char * number, char * empty, user_data * user_data){

    char auxBuffer[AUXBUFFERSIZE] = "../mails/";
    strcat(auxBuffer,user_data->login_info.username);
    
    DIR *directoryPtr;
    directoryPtr = opendir(auxBuffer);
    if (directoryPtr == NULL) {//todo improve
        return COMMANDERROR;
    }

    if ( number != NULL){
        return recoverSpecificMail(number,user_data,directoryPtr);
    } else {
        return getAllMails(directoryPtr,user_data);
    }


    closedir(directoryPtr);
    return 0;
}

executionStatus emptyFunction(char * arg1, char * empty, user_data * user_data){
    log(INFO, "%s", "executing empty functions");
    return COMMANDCOMPLETED;
}

executionStatus noop(char * unused, char * unused2, user_data * user_data){
    char * msg = "+OK\r\n";

    return writeToOutputBuffer(msg, user_data);
}

executionStatus dele(char * toDelete, char * unused, user_data * user_data){ //todo check valid number
    int toDeleteNumber = atoi(toDelete);
    queue(user_data->mailsToDelete, toDeleteNumber);
    return 0;
}

executionStatus rset(char * unused, char * unused2, user_data * user_data){
    freeQueue(user_data->mailsToDelete);
    user_data->mailsToDelete = newQueue();
    return 0;
}

executionStatus quit(char * unused, char * unused2, user_data* user_data){
    if(user_data->session_state == TRANSACTION){
        //execute all functions saved
        //int mailNo;
        toBegin(user_data->mailsToDelete);
        while(hasNext(user_data->mailsToDelete)){
            //mailNo = next(user_data->mailsToDelete);
            //todo delete mailNo
        }
    }
    user_data->session_state = UPDATE;
    
    return 0;
}

static bool userMailDirExists(char * username){
    char auxBuffer[AUXBUFFERSIZE] = "../mails/";
    strcat(auxBuffer,username);
    return opendir(auxBuffer) != NULL; //todo error check in opendir
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


/*
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
*/

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
    
//     findFileData(auxBuff,atoi(mailNo),data->login_info.username);
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
