#include "list.h"

#define AUXBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 2048

#define RECOVERERROR -1
int getUserMails(char * username,user_buffer* outputBuffer){

    char auxBuffer[AUXBUFFERSIZE] = "../mails/";
    char output[AUXBUFFERSIZE] = {0};
    strcat(auxBuffer,username);
    
    DIR *directoryPtr;
    struct dirent *entry;
    directoryPtr = opendir(auxBuffer);
    if (directoryPtr == NULL) {
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
    int mailNumber = 1;
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

        if ( !S_ISREG(fileStat.st_mode) ){
            continue;
        }

        off_t fileSize = fileStat.st_size;
        sprintf(auxBuffer,"%d %lld\r\n",mailNumber,(long long)fileSize);
        strcat(output,auxBuffer);
        mailNumber++;
    }

    if ( errno != 0 ){
        errno = EBADF;//bad file descriptor
        return RECOVERERROR;
    }

    // Close the directory
    closedir(directoryPtr);

    return 0;
}