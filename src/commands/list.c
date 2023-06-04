#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#define PATHBUFFERSIZE 256

#define ERROR -1

int getUserMails(char * username){

    char directoryPath[PATHBUFFERSIZE] = "../mails/";
    strcat(directoryPath,username);
    
    
    DIR *directoryPtr;
    struct dirent *entry;
    directoryPtr = opendir(directoryPath);
    if (directoryPtr == NULL) {
        errno = ENOENT;//error no entry
        return ERROR;
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
    while ((entry = readdir(directoryPtr)) != NULL) {
        // Check if the current entry is a file
        if (entry->d_type != DT_REG ) {
            continue;
        }

    }

    if ( errno != 0 ){
        errno = EBADF;//bad file descriptor
        return ERROR;
    }

    // Close the directory
    closedir(directoryPtr);

    return 0;
}