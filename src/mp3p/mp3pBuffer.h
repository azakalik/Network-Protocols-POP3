
#ifndef MP3PUTILS
#define MP3PUTILS


#define MP3PBUFFSIZE 512


typedef enum {
    WAITFIRSTLINE,
    WAITLETTERCOMMAND,
    WAITSPACE,
    WAITUSERNAME,
    WAITPASSWORD,
    WAITNULLTERMINATION,
    INVALIDCOMMAND,
    WAITSECONDSPACE,
    COMMANDCOMPLETED
} mp3p_states;



typedef struct {
    char bufferData[MP3PBUFFSIZE];
    int currentPosition;
    int length;
    mp3p_states state;
} mp3p_buffer;


typedef mp3p_states (*parse_strategy)(mp3p_buffer * buff);


typedef struct mp3p_data {
    mp3p_buffer inputBuffer;
    parse_strategy parseStrategy;
} mp3p_data;





void initialize_mp3p_data(mp3p_data * data);
void reset_mp3p_buffer(mp3p_buffer * buff);
mp3p_states login_parser(mp3p_buffer* buff);

#endif