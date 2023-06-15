#include "mailsCache.h"
#define MAXFILENAME 256
#define ERROR -1
#define BASEPATH "../mails/"
#define MAILDIRPATHSIZE 256
#define MAXFILEPATHSIZE MAXFILENAME+MAILDIRPATHSIZE
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

typedef struct mail {
    char filename[MAXFILENAME];
    long size_in_bytes;
    bool toDelete;
} mail;

typedef struct mailCache {
    int mailCount;
    int iterator;
    char * maildirPath;
    mail * mails;
} mailCache;

static int getMailCount();
static int validMailNo(mailCache * mailCache, int mailNo);
static int initMailArray(mailCache * mailCache);
static long getMailSize(mailCache * mailCache, int mailNo);

mailCache * initCache(char * username){
    mailCache * mailCache = malloc(sizeof(struct mailCache));
    // obtain path to users maildir
    mailCache->maildirPath = malloc(MAXFILENAME);
    sprintf(mailCache->maildirPath, "./mails/%s/", username);

    mailCache->mailCount = getMailCount(mailCache->maildirPath);
    mailCache->iterator = 0;
    if(mailCache->mailCount > 0)
        mailCache->mails = malloc(sizeof(mail)*mailCache->mailCount);
    else
        mailCache->mails = NULL;
    initMailArray(mailCache);

    return mailCache;
}

void freeCache(mailCache * mailCache){
    if(mailCache != NULL){
        free(mailCache->maildirPath);
        free(mailCache->mails);
    }
    free(mailCache);
}

int openMail(mailCache * mailCache, int mailNo){
    if(mailCache == NULL || !validMailNo(mailCache, mailNo))
        return ERROR;

    //todo
    return 0;
}

int markMailToDelete(mailCache * mailCache, int mailNo){
    if(mailCache == NULL || !validMailNo(mailCache, mailNo))
        return ERROR;

    mailCache->mails[mailNo-1].toDelete = true;
    return 0;
}

int resetToDelete(mailCache * mailCache){
    if(mailCache == NULL)
        return ERROR;

    for (int i = 0; i < mailCache->mailCount; i++)
    {
        mailCache->mails[i].toDelete = false;
    }
    return 0;
}

int deleteMarkedMails(mailCache * mailCache){
    if(mailCache == NULL)
        return ERROR;

    char toDelete[MAXFILEPATHSIZE];
    for (int i = 0; i < mailCache->mailCount; i++)
    {
        if(mailCache->mails[i].toDelete){
            toDelete[0] = 0;
            sprintf(toDelete, "%s%s", mailCache->maildirPath, mailCache->mails[i].filename);
            unlink(toDelete);
        }
    }
    
    return 0;
}

//excludes mails marked to be deleted
int getAmountOfMails(mailCache * mailCache){
    if(mailCache == NULL)
        return ERROR;

    int amount = 0;
    for (int i = 0; i < mailCache->mailCount; i++){
        if(!mailCache->mails[i].toDelete)
            amount++;
    }
    return amount;
}

//excludes mails marked to be deleted
long getSizeOfMails(mailCache * mailCache){
    if(mailCache == NULL)
        return ERROR;

    long sizeInBytes = 0;
    struct stat st;
    for (int i = 0; i < mailCache->mailCount; i++){
        if(!mailCache->mails[i].toDelete){
            if (stat(mailCache->maildirPath, &st) != 0)
                return -1;

            // Retrieve the file size from the stat structure
            sizeInBytes += st.st_size;
        }
    }
    return sizeInBytes;
}

// =========== AUXILIARY ==========
static int getMailCount(char * maildirPath){
    DIR *dir;
    struct dirent *entry;
    int file_count = 0;

    // Open the directory
    dir = opendir(maildirPath);
    if (dir == NULL) { //if the directory doesn't exist, the user has no mails
        return 0;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Increment file count
        file_count++;
    }

    // Close the directory
    closedir(dir);
    return file_count;
}

static int validMailNo(mailCache * mailCache, int mailNo){
    return mailNo > 0 && mailNo <= mailCache->mailCount;
}

static int initMailArray(mailCache *mailCache) {
    DIR *dir;
    struct dirent *entry;
    int file_count = 0;

    // Open the directory
    dir = opendir(mailCache->maildirPath);
    if (dir == NULL) {
        mailCache->mails = NULL;
        return ERROR;
    }

    // Read directory entries
    while ((entry = readdir(dir)) != NULL) {
        // Skip "." and ".." entries
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        // Set the filename
        strncpy(mailCache->mails[file_count].filename, entry->d_name, MAXFILENAME);

        // Set toDelete flag to false
        mailCache->mails[file_count].toDelete = false;

        mailCache->mails[file_count].size_in_bytes = getMailSize(mailCache, file_count+1);

        // Increment file count
        file_count++;

    }

    // Close the directory
    closedir(dir);
    return 0;
}

static long getMailSize(mailCache * mailCache, int mailNo){
    if(mailCache == NULL || !validMailNo(mailCache, mailNo))
        return ERROR;

    char buffer[MAXFILENAME];
    buffer[0] = 0;
    snprintf(buffer, MAXFILEPATHSIZE, "%s/%s", mailCache->maildirPath, mailCache->mails[mailNo-1].filename);
    struct stat st;
    if (stat(mailCache->maildirPath, &st) != 0)
        return -1;

    // Retrieve the file size from the stat structure
    return st.st_size;
}