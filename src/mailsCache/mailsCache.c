#include "mailsCache.h"

#define BASEPATH "../mails/"
#define MAILDIRPATHSIZE 256
#define MAXFILEPATHSIZE MAXFILENAME+MAILDIRPATHSIZE
#define COMMANDSIZE MAXFILEPATHSIZE + 256
#include <stdlib.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include "../logger/logger.h"

typedef struct mail {
    char filename[MAXFILENAME];
    long size_in_bytes;
    bool toDelete;
} mail;

typedef struct retrState {
    FILE * currentMail;
} retrState;

typedef struct listState {
    int iterator;
    mailInfo mailInfo;
} listState;

typedef struct mailCache {
    int mailCount;
    int markedToDelete; //quantity of mails marked to delete
    retrState retrState;
    listState listState;
    char * maildirPath;
    mail * mails;
} mailCache;

static int getMailCount();
static int validMailNo(mailCache * mailCache, int mailNo);
static int initMailArray(mailCache * mailCache);
static long getMailSize(mailCache * mailCache, int mailNo);
static void getPathForMail(mailCache * mailCache, char * buffer, int mailNo);
static void getCommandForPopen(mailCache * mailCache, char * buffer, int mailNo);

mailCache * initCache(char * username){
    mailCache * mailCache = calloc(1, sizeof(struct mailCache));
    // obtain path to users maildir
    mailCache->maildirPath = malloc(MAXFILENAME);
    sprintf(mailCache->maildirPath, "./mails/%s", username);

    mailCache->mailCount = getMailCount(mailCache->maildirPath);
    if(mailCache->mailCount > 0)
        mailCache->mails = malloc(sizeof(mail)*mailCache->mailCount);
    else
        mailCache->mails = NULL;
    initMailArray(mailCache);

    return mailCache;
}

void freeCache(mailCache * mailCache){
    if(mailCache == NULL)
        return;
    closeMail(mailCache); //if there was any mail open, it is closed
    free(mailCache->maildirPath);
    free(mailCache->mails);
    free(mailCache);
}

int openMail(mailCache * mailCache, int mailNo){
    if(mailCache == NULL || !validMailNo(mailCache, mailNo))
        return FAILED;

    closeMail(mailCache); //if another mail (fd) was open, we close it first
    char commandBuffer[COMMANDSIZE];
    getCommandForPopen(mailCache, commandBuffer, mailNo);
    FILE * mailFile = popen(commandBuffer, "r");
    mailCache->retrState.currentMail = mailFile;

    return mailFile != NULL ? fileno(mailFile) : -1;
}

executionStatus getNCharsFromMail(mailCache * mailCache, int * characters, char * buffer){
    if(mailCache == NULL || mailCache->retrState.currentMail == NULL || buffer == NULL || *characters <= 0){
        if(buffer != NULL)
            buffer[0] = 0;
        return FAILED;
    }
    
    *characters = fread(buffer, sizeof(char), *characters, mailCache->retrState.currentMail); //todo make non-blocking

    if(feof(mailCache->retrState.currentMail))
        return FINISHED;
    else if(ferror(mailCache->retrState.currentMail))
        return FAILED;
    else
        return NOT_FINISHED;
}

int closeMail(mailCache * mailCache){
    if(mailCache == NULL || mailCache->retrState.currentMail == NULL)
        return FAILED;

    pclose(mailCache->retrState.currentMail); //todo check if pclose or fclose
    mailCache->retrState.currentMail = NULL;
    return 0;
}

int toBegin(mailCache * mailCache){
    if(mailCache == NULL)
        return FAILED;
    mailCache->listState.iterator = 0;
    return 0;
}

int hasNext(mailCache * mailCache){
    if(mailCache == NULL)
        return FAILED;
    return mailCache->listState.iterator < mailCache->mailCount;
}

int next(mailCache * mailCache, mailInfo * mailInfo){
    if (mailCache == NULL || mailInfo == NULL || !hasNext(mailCache))
        return FAILED;

    mail mail = mailCache->mails[mailCache->listState.iterator];
    mailCache->listState.iterator += 1;
    
    if(mail.toDelete){
        return next(mailCache, mailInfo);
    }

    
    return getMailInfo(mailCache, mailCache->listState.iterator, mailInfo);
}

int getMailInfo(mailCache * mailCache, int mailNo, mailInfo * mailInfo){
    if(mailCache == NULL || mailInfo == NULL || !validMailNo(mailCache, mailNo))
        return FAILED;

    mail mail = mailCache->mails[mailNo-1];

    strcpy(mailInfo->filename, mail.filename);
    mailInfo->sizeInBytes = mail.size_in_bytes;
    mailInfo->mailNo = mailNo;
    return 0;
}

int markMailToDelete(mailCache * mailCache, int mailNo){
    if(mailCache == NULL || !validMailNo(mailCache, mailNo))
        return FAILED;

    mailCache->mails[mailNo-1].toDelete = true;
    mailCache->markedToDelete++;
    return 0;
}

int resetToDelete(mailCache * mailCache){
    if(mailCache == NULL)
        return FAILED;

    for (int mailNo = 0; mailNo < mailCache->mailCount; mailNo++)
    {
        mailCache->mails[mailNo].toDelete = false;
    }
    mailCache->markedToDelete = 0;
    return 0;
}

int deleteMarkedMails(mailCache * mailCache){
    if(mailCache == NULL)
        return FAILED;

    char mailToDelete[MAXFILEPATHSIZE];
    for (int mailNo = 0; mailNo < mailCache->mailCount; mailNo++)
    {
        if(mailCache->mails[mailNo].toDelete){
            getPathForMail(mailCache, mailToDelete, mailNo+1);
            unlink(mailToDelete);
        }
    }
    
    return 0;
}

//excludes mails marked to be deleted
int getAmountOfMails(mailCache * mailCache){
    if(mailCache == NULL)
        return FAILED;

    return mailCache->mailCount - mailCache->markedToDelete;
}

//excludes mails marked to be deleted
long getSizeOfMails(mailCache * mailCache){
    if(mailCache == NULL)
        return FAILED;

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
    return mailNo > 0 && mailNo <= mailCache->mailCount && !mailCache->mails[mailNo-1].toDelete;
}

static int initMailArray(mailCache *mailCache) {
    DIR *dir;
    struct dirent *entry;
    int file_count = 0;

    // Open the directory
    dir = opendir(mailCache->maildirPath);
    if (dir == NULL) {
        mailCache->mails = NULL;
        return FAILED;
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
        return FAILED;

    char buffer[MAXFILEPATHSIZE];
    getPathForMail(mailCache, buffer, mailNo);
    struct stat st;
    if (stat(buffer, &st) != 0)
        return -1;

    // Retrieve the file size from the stat structure
    return st.st_size;
}

//puts in buffer the path to a mail
static void getPathForMail(mailCache * mailCache, char * buffer, int mailNo){
    snprintf(buffer, MAXFILEPATHSIZE, "%s/%s", mailCache->maildirPath, mailCache->mails[mailNo-1].filename);
}

static void getCommandForPopen(mailCache * mailCache, char * buffer, int mailNo){
    char filePath[MAXFILEPATHSIZE];
    getPathForMail(mailCache, filePath, mailNo);

    strcpy(buffer, "cat ");
    strcat(buffer, filePath);
    strcat(buffer, " | sed '1!s/^\\./../g'");
}