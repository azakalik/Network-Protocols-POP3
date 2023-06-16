#define AUXBUFFERSIZE 512
#define OUTPUTBUFFERSIZE 512
#define RECOVERERROR -1
#define GREETINGMESSAGE "+OK Pop3 Server Ready\r\n"
#define WRITE_ERROR -1
#define WRITE_SUCCESS 0
#define STATBUFFERSIZE 32


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
#include "../mailsCache/mailsCache.h"

//----------------FUNCTION-PROTOTYPES--------------------------
executionStatus checkValidUsername(char * username, char * empty, user_data * data);
executionStatus checkValidPassword(char * password, char * empty, user_data * data);
executionStatus emptyFunction(char * arg1, char * empty, user_data * user_data);
executionStatus quit(char *, char *, user_data * user_data);
executionStatus noop(char *, char *, user_data * user_data);
executionStatus dele(char * toDelete, char *, user_data * user_data);
executionStatus rset(char *, char *, user_data * user_data);
executionStatus _stat(char *, char *, user_data * user_data);
executionStatus list(char *, char *, user_data * user_data);
executionStatus retr(char *, char *, user_data * user_data);


//---------------- LIST-OF-COMMANDS----------------------------
#include "popFunctions.h"
#include "strings.h"
command_with_state validCommands[TOTALCOMMANDS] = {
    {"TOP",  emptyFunction,         TRANSACTION},
    {"USER", checkValidUsername,    AUTHENTICATION},
    {"PASS", checkValidPassword,    AUTHENTICATION},
    {"STAT", _stat,                 TRANSACTION},
    {"LIST", list,                  TRANSACTION},
    {"RETR", retr,                  TRANSACTION},
    {"DELE", dele,                  TRANSACTION},
    {"NOOP", noop,                  TRANSACTION},
    {"RSET", rset,                  TRANSACTION},
    {"QUIT", quit,                  ANY},
    {"CAPA", emptyFunction,         ANY},
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


static void sendTerminationCRLF(user_data * user_data, bool addInitialCRLF){
    char * message;
    if(addInitialCRLF) //to use in RETR
        message = "\r\n.\r\n";
    else //to use in LIST
        message = ".\r\n";
    writeToOutputBuffer(message, user_data);
}



//------------------------USER FUNCTION------------------------------------------------------------------------

executionStatus checkValidUsername(char * username, char * empty, user_data * data){
    char * message = "+OK User received.\r\n";
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
        data->mailCache = initCache(data->login_info.username);
        data->session_state = TRANSACTION;
    } else {
        message = "-ERR Authentication failed\r\n";
    }
    int len = strlen(message);
    if ( getBufferFreeSpace(&data->output_buff) >= len){
        writeDataToBuffer(&data->output_buff,message,len);
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

executionStatus emptyFunction(char * arg1, char * empty, user_data * user_data){
    log(INFO, "%s", "executing empty functions");
    return COMMANDCOMPLETED;
}

executionStatus noop(char * unused, char * unused2, user_data * user_data){
    return writeToOutputBuffer("+OK\r\n", user_data);
}

executionStatus dele(char * toDelete, char * unused, user_data * user_data){ //todo check valid number
    int toDeleteNumber = atoi(toDelete);
    char * msg;
    if(markMailToDelete(user_data->mailCache, toDeleteNumber) == 0)
        msg = "+OK message marked to delete\r\n";
    else
        msg = "-ERR That email doesn't seem to exist\r\n";

    writeToOutputBuffer(msg, user_data);
    return 0;
}

executionStatus rset(char * unused, char * unused2, user_data * user_data){
    char * msg;
    if(resetToDelete(user_data->mailCache) == 0)
        msg = "+OK messages no longer marked to delete\r\n";
    else
        msg = "-ERR problem while trying to execute rset\r\n";

    writeToOutputBuffer(msg, user_data);
    return 0;
}

executionStatus quit(char * unused, char * unused2, user_data* user_data){
    char * msg;
    if(user_data->session_state == TRANSACTION){
        if(deleteMarkedMails(user_data->mailCache) == 0)
            msg = "+OK deleting mails and exiting\r\n";
        else
            msg = "-ERR there was a problem deleting your emails\r\n";
    }
    writeToOutputBuffer(msg, user_data);
    user_data->session_state = UPDATE;
    
    return 0;
}

executionStatus _stat(char * unused, char * unused2, user_data * user_data){
    char msg[STATBUFFERSIZE];
    snprintf(msg, STATBUFFERSIZE, "+OK %d %ld\r\n", getAmountOfMails(user_data->mailCache), getSizeOfMails(user_data->mailCache));
    writeToOutputBuffer(msg, user_data);
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

//todo que permita listas grandes
executionStatus list(char * mailNo, char * unused, user_data * user_data){
    char buffer[AUXBUFFERSIZE];
    mailInfo * mailInfo = malloc(sizeof(struct mailInfo));

    //calling list with an argument
    if(mailNo != NULL && mailNo[0] != 0){
        if(getMailInfo(user_data->mailCache, atoi(mailNo), mailInfo) < 0)
            sprintf(buffer, "-ERR That message doesn't seem to exist\r\n");
        else
            sprintf(buffer, "+OK %d %ld\r\n", mailInfo->mailNo, mailInfo->sizeInBytes);

        writeToOutputBuffer(buffer, user_data);
        goto finally;
    }

    //calling list without an argument
    sprintf(buffer, "+OK There are %d messages available\r\n", getAmountOfMails(user_data->mailCache)); //todo 5
    writeToOutputBuffer(buffer, user_data);
    
    toBegin(user_data->mailCache);
    while(hasNext(user_data->mailCache)){
        if(next(user_data->mailCache, mailInfo) >= 0){
            sprintf(buffer, "%d %ld\r\n", mailInfo->mailNo, mailInfo->sizeInBytes);
            writeToOutputBuffer(buffer, user_data);
        }
    }
    sendTerminationCRLF(user_data, false);
finally:
    free(mailInfo);
    return 0;
}

executionStatus retr(char * mailNoString, char * unused, user_data * user_data){
    char buffer[AUXBUFFERSIZE+1];
    int mailNo = atoi(mailNoString);
    char * message = "+OK message follows\r\n";
    writeToOutputBuffer(message, user_data);
    if(openMail(user_data->mailCache, mailNo) < 0)
        return COMMANDERROR;
    int saved = getNCharsFromMail(user_data->mailCache, getBufferFreeSpace(&user_data->output_buff), buffer);
    buffer[saved] = 0;
    writeToOutputBuffer(buffer, user_data);
    sendTerminationCRLF(user_data, true);
    closeMail(user_data->mailCache);
    return 0;
}