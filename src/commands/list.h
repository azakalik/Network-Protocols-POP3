
#ifndef LIST
#define LIST

#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <unistd.h>
#include "../server/serverFunctions.h"
#include "../logger/logger.h"
int getUserMails(char * username,user_buffer *userBuffer);

#endif