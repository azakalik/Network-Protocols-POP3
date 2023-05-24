#ifndef _COMMON_H_
#define _COMMON_H_

#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h> //for variable argument functions
#include <errno.h>


// === functions inspired by Jacob Sorber ===
char * bin2hex(const unsigned char * input, size_t len); //for very basic logging
/// =========================================

#endif