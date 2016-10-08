
#include "uuuid2.h"

#ifndef CP_ICACHE_FLASH_ATTR
#define CP_ICACHE_FLASH_ATTR __attribute__(())
#endif

#ifdef __FreeBSD__
#include </usr/include/uuid.h>
bool uuuid2_gen(uuuid2_t *u)
{
	uuid_t uuid = {0};
	uint32_t st = 0;
	uuid_create(&uuid, &st);
	if (st != uuid_s_ok) 
		return 0;

	uint8_t * uuid_b = (uint8_t*)&uuid;
	for(uint8_t i = 0; i < 16; ++i)
		u->bytes[i] = uuid_b[i];
	return 1;
}
#endif


#ifdef __linux__
#include <uuid/uuid.h>

bool uuuid2_gen(uuuid2_t *u)
{
	uuid_t uuid = {0};
	uuid_generate_time(uuid);

	uint8_t * uuid_b = (uint8_t*)&uuid;
	for(uint8_t i = 0; i < 16; ++i)
		u->bytes[i] = uuid_b[i];
	return 1;
}

#endif


#ifdef __APPLE__
#include <stdlib.h>
#include <uuid/uuid.h>

bool uuuid2_gen(uuuid2_t *u)
{
	uuid_t uuid = {0};
	uuid_generate_time(uuid);

	uint8_t * uuid_b = (uint8_t*)&uuid;
	for(uint8_t i = 0; i < 16; ++i)
		u->bytes[i] = uuid_b[i];
	return 1;
}

#endif


#ifdef ESP8266
#include <c_types.h>
#include "user_interface.h"
#include <osapi.h>
#include "ets_sys.h"

uint64_t sec = 0;
uint32_t frac = 0;
char hwaddr[6] = {0};
uint64_t _speical_time = 0xD1B21DD213814000LL;

#ifndef __ESP8266_NO_TIMESTAMP__
extern time_t ntp_unix_timestamp;
#endif 

#define STATIC_TIME 1475815201

void CP_ICACHE_FLASH_ATTR arc4random_buf(unsigned char *b, int len )
{
	int i;
	for (i=0; i<len; i++ ) {
		b[i] = (unsigned char)os_random() % 255;	
	}
}


bool CP_ICACHE_FLASH_ATTR uuuid2_gen(uuuid2_t *u)
{
//	uuid_t uuid = {0};
	uint8_t i;
	if ( hwaddr[0] == 0 ) {
		wifi_get_macaddr(STATION_IF, hwaddr);
	}

//	struct bintime bt;

	// bintime(&bt);
#ifdef __ESP8266_NO_TIMESTAMP__
	if ( sec == 0 ) {
		sec = STATIC_TIME + os_random();
		sec=sec+_speical_time;
	} 
#else
	if (ntp_unix_timestamp == 0) 
	{		
		sec = STATIC_TIME + os_random();
	} else {
		sec = ntp_unix_timestamp;
	}	
#endif

	if (frac == 0 ) {
		uint8_t*fp = (uint8_t*)&frac;
		for (i=0; i<4; i++ ) {
			fp[i] = (unsigned char)os_random() % 255;	
		}
		frac+=system_get_time();
	} else {
		frac++;
		if (frac == 0) 
		{
			frac=1;
#ifdef __ESP8266_NO_TIMESTAMP__
			sec++;
#endif
		}
	}
	
	for(i = 1; i < 7; ++i) {
		u->bytes[16-i] = (uint8_t)hwaddr[6-i];
	}	

	uint8_t *frac_array = (uint8_t*)&frac;
	u->bytes[9] = frac_array[0];
	u->bytes[8] = frac_array[1];
	u->bytes[7] = frac_array[2];
	u->bytes[6] = frac_array[3];
	uint8_t *sec_array = (uint8_t*)&sec;
	u->bytes[5] = sec_array[0];
	u->bytes[4] = sec_array[1];
	u->bytes[3] = sec_array[2];
	u->bytes[2] = sec_array[3];
	u->bytes[1] = sec_array[4];
	u->bytes[0] = sec_array[5];

	return 1;
}

#endif


bool uuuid2_gen_nil (uuuid2_t * u) 
{
	int i=0;
	if ( sizeof(u->bytes) == 16 ) {
		return 0;
	} else {
		for(i = 0; i < 16; ++i) 
		{
			u->bytes[i] = 0;
		}
	}

}


bool CP_ICACHE_FLASH_ATTR  uuuid2_eq(uuuid2_t *a, uuuid2_t *b) {
	int r;
	r = memcmp(a->bytes, b->bytes, sizeof(a->bytes));
	if ( r == 0 ) {
		return 1;
	} else {
		return 0;
	}
}

bool CP_ICACHE_FLASH_ATTR  uuuid2_copy(uuuid2_t *from, uuuid2_t *to) {
	if ( sizeof(from->bytes) != sizeof(to->bytes) ) 
	{ 
		CHATFABRIC_DEBUG(1, " Size Mismatch" );

		return 0; 
		
	} else { 	
		memcpy(to->bytes, from->bytes, sizeof(from->bytes));
		return 1;
	}
}
	

bool CP_ICACHE_FLASH_ATTR uuuid2_to_str(char * buf, size_t size, uuuid2_t *id)
{
#ifdef ESP8266
	return sprintf(
		buf,
#else 
	return snprintf(
		buf, size,
#endif
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		id->bytes[0], id->bytes[1], id->bytes[2], id->bytes[3],
		id->bytes[4], id->bytes[5],
		id->bytes[6], id->bytes[7],
		id->bytes[8], id->bytes[9],
		id->bytes[10], id->bytes[11], id->bytes[12], id->bytes[13], id->bytes[14], id->bytes[15]) + 1 <= size;
}
#ifdef ESP8266
bool CP_ICACHE_FLASH_ATTR uuuid2_from_str(uuuid2_t * id, const char * buf)
{
	// esp-sdk does not have a scanf function.
	return 0;
}
#else 
bool CP_ICACHE_FLASH_ATTR uuuid2_from_str(uuuid2_t * id, const char * buf)
{
	uint8_t i;
	int b[16] = {0};
	bool valid = sscanf(
		buf,
		"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
		b + 0, b + 1, b + 2, b + 3,
		b + 4, b + 5,
		b + 6, b + 7,
		b + 8, b + 9,
		b + 10, b + 11, b + 12, b + 13, b + 14, b + 15) == 16;
	for(i = 0; i < 16; ++i)
		if(b[i] >= 0 && b[i] <= 0xff)
			id->bytes[i] = (uint8_t)b[i];
		else
			return false;
	return valid;
}
#endif

