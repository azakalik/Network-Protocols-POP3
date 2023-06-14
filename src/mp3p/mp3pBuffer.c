#include "mp3pBuffer.h"



static mp3p_states parse_login_character(char c,mp3p_states state){
    if (state == WAITFIRSTLINE){
        if ( c == '\n')
            return WAITLETTERCOMMAND;
        return WAITFIRSTLINE;
    }

    if (state == WAITLETTERCOMMAND){
        if ( c == 'u' || c == 'U')
            return WAITSPACE;
        return INVALIDCOMMAND;
    }

    if (state == WAITSPACE){
        if ( c == ' ')
            return WAITUSERNAME;
        return INVALIDCOMMAND;
    }

    if ( state == WAITPASSWORD ){
        if ( c == ' ')
            return WAITSECONDSPACE;
        return WAITPASSWORD;
    }

    if ( state == WAITSECONDSPACE){
        if ( c == ' ')
            return WAITPASSWORD;
        return INVALIDCOMMAND;
    }

    if (state == WAITPASSWORD){
        if ( c == '\0')
            return COMMANDCOMPLETED;
        return WAITPASSWORD;
    }

    return INVALIDCOMMAND;
}

mp3p_states login_parser(mp3p_buffer* buff){

    for ( int i = buff->currentPosition; i < buff->length ;i++){
        char c = buff->bufferData[i];    
        buff->state = parse_login_character(c,buff->state);
    }


    return buff->state;
}





void initialize_mp3p_data(mp3p_data * data){
    reset_mp3p_buffer(&data->inputBuffer);
    data->parseStrategy = login_parser;
    data->inputBuffer.state = WAITFIRSTLINE;
}




void reset_mp3p_buffer(mp3p_buffer * buff){
    buff->currentPosition = 0;
    buff->length = MP3PBUFFSIZE;
}




