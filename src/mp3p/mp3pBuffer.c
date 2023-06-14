#include "mp3pBuffer.h"


mp3p_states parse_login_character(char c,mp3p_states state){
    return 0;
}

mp3p_states login_parser(mp3p_data* data){
    for ( int i = 0; i < data->inputBuffer.length ;i++){
        char c = data->inputBuffer.bufferData[i];
        
        data->state = parse_login_character(c,data->state);
    }
}





void initialize_mp3p_data(mp3p_data * data){
    reset_mp3p_buffer(data);
    data->parseStrategy = login_parser;
}




void reset_mp3p_buffer(mp3p_buffer * buff){
    buff->currentPosition = 0;
    buff->length = MP3PBUFFSIZE;
}




