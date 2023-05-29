#include "popFunctions.h"
#define GREETINGMESSAGE "+OK Pop3 Server Ready\r\n"

void sendGreeting(user_data * user){
    char * greetingMessage = GREETINGMESSAGE;
    writeDataToBuffer(&user->output_buff,greetingMessage,strlen(greetingMessage));
}