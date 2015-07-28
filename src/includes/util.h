#include <sys/types.h>
#include <string.h> //memcpy
#include <stdio.h>
#include <stdlib.h>
#include "__attribute__.h"


#ifndef CF_UTIL_H
#define CF_UTIL_H



void util_print_bin2hex(unsigned char * x, int len);

void 
util_hexprint ( unsigned char *str, uint32_t len );

void
util_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght );



#endif
