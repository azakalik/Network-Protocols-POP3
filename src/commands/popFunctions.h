
#ifndef POP_FUNCTIONS
#define POP_FUNCTIONS

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../server/serverFunctions.h"
#include "../logger/logger.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "fileBuffer.h"


void sendGreeting(user_data * user);
int getUserMails(char * username, user_data * data);
int writeToOutputBuffer(char * buffer, user_data* data );
int emptyFunction(char * arg1, char * arg2);
int retr(char * username, char * msgNum, user_data* data);


#endif