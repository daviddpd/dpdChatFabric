#ifndef UUUID2_H
#define UUUID2_H


#ifndef ESP8266
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#else
#include "ets_sys.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"
#include "__attribute__.h"
#define printf(...) os_printf( __VA_ARGS__ )
#define sscanf os_scanf
#define sprintf(...) os_sprintf( __VA_ARGS__ )
#endif

typedef struct
{
	uint8_t bytes[16];
} uuuid2_t;

bool uuuid2_gen(uuuid2_t * u);
bool uuuid2_gen_nil(uuuid2_t * u);
bool uuuid2_eq(uuuid2_t *a, uuuid2_t *b);
bool uuuid2_copy(uuuid2_t *from, uuuid2_t *to);

bool uuuid2_to_str(char * buf, size_t size, uuuid2_t *id);
bool uuuid2_from_str(uuuid2_t * id, const char * buf);


#endif