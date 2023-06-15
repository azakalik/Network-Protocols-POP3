#ifndef MP3PPARSER
#define MP3PPARSER


#define VERSIONSIZE 128
#define AUTHORIZATION 64
#define COMMANDLEN 256

typedef void (*command_strategy)(void * arg);

typedef struct {
    char version[VERSIONSIZE];
    char authorization[AUTHORIZATION];
    char command[COMMANDLEN];
    command_strategy commandFunction;
} mp3p_data;


#define DGRAMSUCCESS 1
#define DGRAMERROR 0

#endif