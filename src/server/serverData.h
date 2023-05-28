
#include <stdbool.h>
#define BUFFERSIZE 1024

typedef enum {
    GREETING,
    AUTHENTICATION,
    TRANSACTION,
    UPDATE,
} server_state;

typedef struct {
    char buffer[BUFFERSIZE];
    bool full;
    int read_ptr;
    int write_ptr;
} user_buffer;


typedef struct {    
    user_buffer entry_buff;
    user_buffer output_buff;
    server_state session_state;
    int socket;

} user_data;



#define ACCEPT_FAILURE -1