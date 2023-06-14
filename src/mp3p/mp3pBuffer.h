
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
} mp3p_states;


typedef mp3p_states (*parse_strategy)(mp3p_data * buff);

typedef struct {
    char bufferData[MP3PBUFFSIZE];
    int currentPosition;
    int length;
} mp3p_buffer;


typedef struct {
    mp3p_buffer inputBuffer;
    parse_strategy parseStrategy;
    mp3p_states state;
} mp3p_data;




#endif