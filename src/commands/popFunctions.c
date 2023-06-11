#define AUXBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 2048
#define RECOVERERROR -1
#define GETNUMBER(n) (n - '0')
#define GREETINGMESSAGE "+OK Pop3 Server Ready\r\n"


//---------------- LIST ----------------------------
#include "popFunctions.h"
#include "strings.h"
valid_command_list validCommands[TOTALCOMMANDS] = {
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
command_handler getCommand(char * command_name){
    for ( int i = 0; i < TOTALCOMMANDS; i++){
        if ( strcasecmp(validCommands[i].commandStr, command_name) == 0 ){
            return validCommands[i].execute_command;
        }
    }
    return NULL;
}

//-------------------------LIST FUNCTIONS-----------------------------------------------------------------------

void sendGreeting(user_data * user){
    char * greetingMessage = GREETINGMESSAGE;
    writeDataToBuffer(&user->output_buff,greetingMessage,strlen(greetingMessage));
}

/*
static int parseMailNumber(char * fileName){
    int number = 0;
    for ( int i = 0; fileName[i] >= '0' && fileName[i] <= '9';i++){
        int digit = GETNUMBER(fileName[i]);
        number = (number * 10) + digit;
    }
    return number;
}
*/


int getUserMails(char * username,user_buffer* outputBuffer){

    char auxBuffer[AUXBUFFERSIZE] = "../mails/";
    char output[OUTPUTBUFFERSIZE] = {0};
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
    errno = 0;
    struct stat fileStat;
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
            strcat(output,auxBuffer);
            mailNumber += 1;
        }
    }

    if ( errno != 0 ){
        errno = EBADF;//bad file descriptor
        return RECOVERERROR;
    }

    
    strcat(output,".\r\n");
    // Close the directory
    closedir(directoryPtr);

    return 0;
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
int retr(char * username, char * msgNum, user_buffer *userBuffer){

    char auxBuffer[AUXBUFFERSIZE] = "../mails/";
    char output[OUTPUTBUFFERSIZE] = {0};
    //todo: validate msgNum
    sprintf(auxBuffer,"../mails/%s/%s", username, msgNum);
    char * filePath = auxBuffer;


    FILE * file = fopen(filePath, "r");
    if (file == NULL) {
        log(ERROR,"error opening file %s\n",filePath);
        return RECOVERERROR;
    }


    struct stat fileStat;
    if ( stat(filePath, &fileStat) < 0){
            log(ERROR,"error recovering file statitistics for file %s\n",filePath);
            fclose(file);
            return RECOVERERROR;
        }
    
    if ( S_ISREG(fileStat.st_mode) ){
            off_t fileSize = fileStat.st_size;
            sprintf(auxBuffer,"+OK %lld octets \r\n",(long long)fileSize);
            strcat(output,auxBuffer);

            //podria usar filesize en vez de outputbuffersize??
        

             fread(output, sizeof(char), OUTPUTBUFFERSIZE, file);

//fileSize = 14
//bufferdelectura 5
//5 - 5 - 4

            //todo: validation

            //paso1: buffer de lectura
            //paso2: find replace: \r\n.\r\n --> \r\n..\r\n (puede ser q en el primer buffer tenga r\n. y en otro r\n )
            //paso3: copio el buffer de lectura en el userbuffer (salida)

            strcat(output,".\r\n");

            //copiar output en userBuffer ??
            writeDataToBuffer(userBuffer, output, strlen(output));
            //getBufferFreeSpace
            // escribo lo q puedo
            //si no hay espacio en userBuffer --> 
        }
    
    fclose(file);
    return 0;

}
