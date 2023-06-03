//todo esto no sirve, borrar

#ifndef AUTH_PARSERS
#define AUTH_PARSERS

#include <stdio.h>
#include <string.h>


typedef enum {
    USER
} parser_type;

typedef enum {
    START,
    COMMAND,
    SPACE,
    USERNAME,
    COMPLETED,
    FAILED
} auth_parser_state;


typedef struct {
    parser_type parser;
    
} user_context;

void parse_user_auth_command(char c, auth_parser_state state);

#endif