/* 
Copyright (c) 2015, David P. Discher
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include "uuuid2.h"
#include "__attribute__.h"

#ifdef ESP8266
#include "mem.h"
#define malloc os_malloc
#endif



void CP_ICACHE_FLASH_ATTR uuidToBytes(void *str, uuid_cp *uuid)
{
#ifdef FREEBSD
	uuid_enc_be( str, uuid);
#elif defined(ESP8266)
	uuid_enc_be( str, uuid);
#elif defined(IOS_APP)
	memcpy(str, uuid, 16);
#endif 

	return;

}
void CP_ICACHE_FLASH_ATTR uuidFromBytes(void *str, uuid_cp *uuid)
{
#ifdef FREEBSD
	uuid_dec_be( str, uuid);
#elif defined(ESP8266)
	uuid_dec_be( str, uuid);
#elif defined(IOS_APP)
	memcpy(uuid, str, 16);
#endif 
	return;

}
void CP_ICACHE_FLASH_ATTR uuidToStr(void *str, uuid_cp *uuid)
{
#ifdef FREEBSD
	uint32_t status;
	uuid_to_string(uuid, str, &status);
#elif defined(ESP8266)
	// Not implemented for ESP8266
	str = (char*)malloc(38);
	bzero(str, 38);
	int s = snprintf_uuid(str, 38, uuid);
	os_printf ( " => uuidToStr %d %s \n ", s, str );
#elif defined(IOS_APP)
	uuid_unparse_lower(uuid, str);
#endif 
	return;
}
void CP_ICACHE_FLASH_ATTR uuidFromStr(void *str, uuid_cp *uuid)
{
#ifdef FREEBSD
	uint32_t status;
	uuid_from_string(str, uuid, &status);
#elif defined(ESP8266)
	// ESP8266 sscanf is problematic in this SDK.
#elif defined(IOS_APP)
	// this may not work, untested
	uuid_parse( (char *)str, uuid);
#endif 
	return;
}

void CP_ICACHE_FLASH_ATTR uuidCreate(uuid_cp *uuid)
{
#ifdef FREEBSD
	uint32_t status;
	uuid_create( uuid, &status);
#elif defined(ESP8266)
	uint32_t status;
	uuid_create( uuid, &status);
#elif defined(IOS_APP)
	uuid_generate( uuid );
#endif 

	return;

}
void CP_ICACHE_FLASH_ATTR uuidCreateNil(uuid_cp *uuid)
{
#ifdef FREEBSD
	uint32_t status;
	uuid_create_nil( uuid, &status);
#elif defined(ESP8266)
	uint32_t status;
	uuid_create_nil( uuid, &status);
#elif defined(IOS_APP)
	uuid_clear( uuid );
#endif 
	return;
}


void CP_ICACHE_FLASH_ATTR uuidCopy(uuid_cp *from, uuid_cp *to )
{
#ifdef FREEBSD
	*to = *from;
#elif defined(ESP8266)
	*to = *from;
#elif defined(IOS_APP)
    uuid_copy(to, from);
#endif

}


int CP_ICACHE_FLASH_ATTR uuidCompare(uuid_cp *uuid0, uuid_cp *uuid1) 
{
#ifdef FREEBSD
	uint32_t status;
	return uuid_compare(uuid0, uuid1, &status);
#elif defined(ESP8266)
	uint32_t status;
	return uuid_compare(uuid0, uuid1, &status);
#elif defined(IOS_APP)

	return uuid_compare(uuid0, uuid1);
/*
    unsigned char *tmp1 = malloc(36);
    unsigned char *tmp2 = malloc(36);

    uuid_unparse_lower(uuid0, tmp1);
    uuid_unparse_lower(uuid1, tmp2);
        
    free(tmp1);
    free(tmp2);
    return strncmp(tmp1, tmp2, 36);
*/

#endif

	return -1;
}

