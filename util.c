#include "util.h"
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