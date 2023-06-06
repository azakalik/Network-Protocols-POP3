#include "popFunctions.h"

#define AUXBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 2048
#define RECOVERERROR -1
#define GETNUMBER(n) (n - '0')
#define GREETINGMESSAGE "+OK Pop3 Server Ready\r\n"

void sendGreeting(user_data * user){
    char * greetingMessage = GREETINGMESSAGE;
    writeDataToBuffer(&user->output_buff,greetingMessage,strlen(greetingMessage));
}


static int parseMailNumber(char * fileName){
    int number = 0;
    for ( int i = 0; fileName[i] >= '0' && fileName[i] <= '9';i++){
        int digit = GETNUMBER(fileName[i]);
        number = (number * 10) + digit;
    }
    return number;
}


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
            int mailNumber = parseMailNumber(entry->d_name);
            off_t fileSize = fileStat.st_size;
            sprintf(auxBuffer,"%d %lld\r\n",mailNumber,(long long)fileSize);
            strcat(output,auxBuffer);
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
