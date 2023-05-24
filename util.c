#include "util.h"

void err_n_die(const char * fmt, ...){
    int errno_save;
    va_list ap;

    //any system or library call can set errno, so we save it
    errno_save = errno;

    //print out fmt and args to stdout
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    //print out error message if errno was set
    if (errno_save != 0){
        fprintf(stdout, "(errno = %d) : %s\n", errno_save,
        strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }
    va_end(ap);

    //terminate with error
    exit(1);
}

//converts binary to hex (for logging)
char * bin2hex(const unsigned char * input, size_t len){
    char * result;
    char * hexits = "0123456789ABCDEF";

    if ((input == NULL) || len <= 0)
        return NULL;

    //(2 hexits + space) + NULL
    int resultlength = (len*3)+1;

    result = malloc(resultlength);
    memset(result, 0, resultlength);

    for (size_t i = 0; i < len; i ++){
        result[i*3] = hexits[input[i] >> 4];
        result[(i*3)+1] = hexits[input[i] & 0x0F];
        result[(i*3)+2] = ' ';
    }

    return result;
}