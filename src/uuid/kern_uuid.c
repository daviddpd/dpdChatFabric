/*-
 * Copyright (c) 2002 Marcel Moolenaar
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifdef ESP8266
//#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#ifndef CP_ICACHE_FLASH_ATTR
#define CP_ICACHE_FLASH_ATTR __attribute__(())
#endif
#define ESP_WORD_ALIGN __attribute__ ((aligned (4)))
#else
#define CP_ICACHE_FLASH_ATTR __attribute__(())
#define ESP_WORD_ALIGN __attribute__ (())
#endif

#include <c_types.h>
#include "uuid_local.h"
#include "user_interface.h"
extern time_t ntp_unix_timestamp;
time_t uuid_frac_time = 0;

/*
 * See also:
 *	http://www.opengroup.org/dce/info/draft-leach-uuids-guids-01.txt
 *	http://www.opengroup.org/onlinepubs/009629399/apdxa.htm
 *
 * Note that the generator state is itself an UUID, but the time and clock
 * sequence fields are written in the native byte order.
 */

/* We use an alternative, more convenient representation in the generator. */
struct uuid_private {
	union {
		uint64_t	ll;		/* internal. */
		struct {
			uint32_t	low;
			uint16_t	mid;
			uint16_t	hi;
		} x;
	} time;
	uint16_t	seq;			/* Big-endian. */
	uint16_t	node[UUID_NODE_LEN>>1];
};


struct uuid_macaddr {
	uint16_t	state;
#define	UUID_ETHER_EMPTY	0
#define	UUID_ETHER_RANDOM	1
#define	UUID_ETHER_UNIQUE	2
	uint16_t	node[UUID_NODE_LEN>>1];
};

static struct uuid_private uuid_last;

#define UUID_NETHER	4
static struct uuid_macaddr uuid_ether[UUID_NETHER];


/*
 * Return the first MAC address added in the array. If it's empty, then
 * construct a sufficiently random multicast MAC address first. Any
 * addresses added later will bump the random MAC address up tp the next
 * index.
 */
static void CP_ICACHE_FLASH_ATTR
uuid_node(uint16_t *node)
{
	char hwaddr[6];
	wifi_get_macaddr(STATION_IF, hwaddr);
	node[0] = (hwaddr[1]<< 8) | hwaddr[0];
	node[1] = (hwaddr[3]<< 8) | hwaddr[2];
	node[2] = (hwaddr[5]<< 8) | hwaddr[4];
}

/*
 * Get the current time as a 60 bit count of 100-nanosecond intervals
 * since 00:00:00.00, October 15,1582. We apply a magic offset to convert
 * the Unix time since 00:00:00.00, January 1, 1970 to the date of the
 * Gregorian reform to the Christian calendar.
 */
static uint64_t CP_ICACHE_FLASH_ATTR
uuid_time(void)
{
	struct bintime bt;
	uint64_t time = 0x01B21DD213814000LL;

	// bintime(&bt);
	bt.sec = ntp_unix_timestamp;
	if (ntp_unix_timestamp == 0) 
	{		
		bt.sec = 1443942000 + arc4random();
	} else {
		bt.sec = ntp_unix_timestamp;
	}	
	if ( uuid_frac_time == 0 ) {
		uuid_frac_time = system_get_time() + arc4random();
	} else {
		uuid_frac_time++;
	}
	bt.frac = uuid_frac_time;
	
	time += (uint64_t)bt.sec * 10000000LL;
	time += (10000000LL * (uint32_t)(bt.frac >> 32)) >> 32;
	return (time & ((1LL << 60) - 1LL));
}

struct uuid * CP_ICACHE_FLASH_ATTR
kern_uuidgen(struct uuid *store, size_t count)
{
	struct uuid_private uuid;
	uint64_t time;
	size_t n;

//	mtx_lock(&uuid_mutex);

	uuid_node(uuid.node);
	time = uuid_time();

	if (uuid_last.time.ll == 0LL || uuid_last.node[0] != uuid.node[0] ||
	    uuid_last.node[1] != uuid.node[1] ||
	    uuid_last.node[2] != uuid.node[2])
		uuid.seq = (uint16_t)arc4random() & 0x3fff;
	else if (uuid_last.time.ll >= time)
		uuid.seq = (uuid_last.seq + 1) & 0x3fff;
	else
		uuid.seq = uuid_last.seq;

	uuid_last = uuid;
	uuid_last.time.ll = (time + count - 1) & ((1LL << 60) - 1LL);

//	mtx_unlock(&uuid_mutex);

	/* Set sequence and variant and deal with byte order. */
	uuid.seq = htobe16(uuid.seq | 0x8000);

	for (n = 0; n < count; n++) {
		/* Set time and version (=1). */
		uuid.time.x.low = (uint32_t)time;
		uuid.time.x.mid = (uint16_t)(time >> 32);
		uuid.time.x.hi = ((uint16_t)(time >> 48) & 0xfff) | (1 << 12);
		store[n] = *(struct uuid *)&uuid;
		time++;
	}

	return (store);
}

#ifndef _SYS_SYSPROTO_H_
struct uuidgen_args {
	struct uuid *store;
	int	count;
};
#endif

int CP_ICACHE_FLASH_ATTR
snprintf_uuid(char *buf, size_t sz, struct uuid *uuid)
{
	struct uuid_private *id;
	int cnt;

	id = (struct uuid_private *)uuid;
//	cnt = snprintf(buf, sz, "%08x-%04x-%04x-%04x-%04x%04x%04x",
	cnt = os_sprintf(buf, "%08x-%04x-%04x-%04x-%04x%04x%04x",
	    id->time.x.low, id->time.x.mid, id->time.x.hi, be16toh(id->seq),
	    be16toh(id->node[0]), be16toh(id->node[1]), be16toh(id->node[2]));
	return (cnt);
}

int CP_ICACHE_FLASH_ATTR
printf_uuid(struct uuid *uuid)
{
	char buf[38];

	snprintf_uuid(buf, sizeof(buf), uuid);
	return (printf("%s", buf));
}

int CP_ICACHE_FLASH_ATTR
sbuf_printf_uuid(struct sbuf *sb, struct uuid *uuid)
{
	char buf[38];

	snprintf_uuid(buf, sizeof(buf), uuid);
	return (sbuf_printf(sb, "%s", buf));
}

/*
 * Encode/Decode UUID into byte-stream.
 *   http://www.opengroup.org/dce/info/draft-leach-uuids-guids-01.txt
 *
 * 0                   1                   2                   3
 *   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                          time_low                             |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |       time_mid                |         time_hi_and_version   |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |clk_seq_hi_res |  clk_seq_low  |         node (0-1)            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *  |                         node (2-5)                            |
 *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

void CP_ICACHE_FLASH_ATTR
le_uuid_enc(void *buf, struct uuid const *uuid)
{
	u_char *p;
	int i;

	p = buf;
	le32enc(p, uuid->time_low);
	le16enc(p + 4, uuid->time_mid);
	le16enc(p + 6, uuid->time_hi_and_version);
	p[8] = uuid->clock_seq_hi_and_reserved;
	p[9] = uuid->clock_seq_low;
	for (i = 0; i < _UUID_NODE_LEN; i++)
		p[10 + i] = uuid->node[i];
}

void CP_ICACHE_FLASH_ATTR
le_uuid_dec(void const *buf, struct uuid *uuid)
{
	u_char const *p;
	int i;

	p = buf;
	uuid->time_low = le32dec(p);
	uuid->time_mid = le16dec(p + 4);
	uuid->time_hi_and_version = le16dec(p + 6);
	uuid->clock_seq_hi_and_reserved = p[8];
	uuid->clock_seq_low = p[9];
	for (i = 0; i < _UUID_NODE_LEN; i++)
		uuid->node[i] = p[10 + i];
}

void CP_ICACHE_FLASH_ATTR
be_uuid_enc(void *buf, struct uuid const *uuid)
{
	u_char *p;
	int i;

	p = buf;
	be32enc(p, uuid->time_low);
	be16enc(p + 4, uuid->time_mid);
	be16enc(p + 6, uuid->time_hi_and_version);
	p[8] = uuid->clock_seq_hi_and_reserved;
	p[9] = uuid->clock_seq_low;
	for (i = 0; i < _UUID_NODE_LEN; i++)
		p[10 + i] = uuid->node[i];
}

void CP_ICACHE_FLASH_ATTR
be_uuid_dec(void const *buf, struct uuid *uuid)
{
	u_char const *p;
	int i;

	p = buf;
	uuid->time_low = be32dec(p);
	uuid->time_mid = be16dec(p + 4);
	uuid->time_hi_and_version = be16dec(p + 6);
	uuid->clock_seq_hi_and_reserved = p[8];
	uuid->clock_seq_low = p[9];
	for (i = 0; i < _UUID_NODE_LEN; i++)
		uuid->node[i] = p[10 + i];
}
