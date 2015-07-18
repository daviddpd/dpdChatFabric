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

#include <c_types.h>
#include <driver/uuid.h>
#include "user_interface.h"
extern time_t ntp_unix_timestamp;

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
static void ICACHE_FLASH_ATTR
uuid_node(uint16_t *node)
{
	char hwaddr[6];
	wifi_get_macaddr(STATION_IF, hwaddr);
	node[0] = (hwaddr[0]<< 8) | hwaddr[1];
	node[1] = (hwaddr[2]<< 8) | hwaddr[3];
	node[2] = (hwaddr[4]<< 8) | hwaddr[5];
}

/*
 * Get the current time as a 60 bit count of 100-nanosecond intervals
 * since 00:00:00.00, October 15,1582. We apply a magic offset to convert
 * the Unix time since 00:00:00.00, January 1, 1970 to the date of the
 * Gregorian reform to the Christian calendar.
 */
static uint64_t ICACHE_FLASH_ATTR
uuid_time(void)
{
	struct bintime bt;
	uint64_t time = 0x01B21DD213814000LL;

	// bintime(&bt);
	bt.sec = ntp_unix_timestamp;
	bt.frac = system_get_time();
	
	time += (uint64_t)bt.sec * 10000000LL;
	time += (10000000LL * (uint32_t)(bt.frac >> 32)) >> 32;
	return (time & ((1LL << 60) - 1LL));
}

struct uuid * ICACHE_FLASH_ATTR
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
/*
int
sys_uuidgen(struct thread *td, struct uuidgen_args *uap)
{
	struct uuid *store;
	size_t count;
	int error;

	if (uap->count < 1 || uap->count > 2048)
		return (EINVAL);

	count = uap->count;
	store = malloc(count * sizeof(struct uuid), M_TEMP, M_WAITOK);
	kern_uuidgen(store, count);
	error = copyout(store, uap->store, count * sizeof(struct uuid));
	free(store, M_TEMP);
	return (error);
}

int ICACHE_FLASH_ATTR
uuid_ether_add(const uint8_t *addr)
{
	int i, sum;

	//
	// Validate input. No multicast (flag 0x1), no locally administered
	 // (flag 0x2) and no 'all-zeroes' addresses.
	 //
	if (addr[0] & 0x03)
		return (EINVAL);
	sum = 0;
	for (i = 0; i < UUID_NODE_LEN; i++)
		sum += addr[i];
	if (sum == 0)
		return (EINVAL);

//	mtx_lock(&uuid_mutex);

	// Make sure the MAC isn't known already and that there's space. 
	i = 0;
	while (i < UUID_NETHER && uuid_ether[i].state == UUID_ETHER_UNIQUE) {
		if (!bcmp(addr, uuid_ether[i].node, UUID_NODE_LEN)) {
			//mtx_unlock(&uuid_mutex);
			return (EEXIST);
		}
		i++;
	}
	if (i == UUID_NETHER) {
		//mtx_unlock(&uuid_mutex);
		return (ENOSPC);
	}

	// Insert MAC at index, moving the non-empty entry if possible. 
	if (uuid_ether[i].state == UUID_ETHER_RANDOM && i < UUID_NETHER - 1)
		uuid_ether[i + 1] = uuid_ether[i];
	uuid_ether[i].state = UUID_ETHER_UNIQUE;
	bcopy(addr, uuid_ether[i].node, UUID_NODE_LEN);
	//mtx_unlock(&uuid_mutex);
	return (0);
}

int ICACHE_FLASH_ATTR
uuid_ether_del(const uint8_t *addr)
{
	int i;

	//mtx_lock(&uuid_mutex);
	i = 0;
	while (i < UUID_NETHER && uuid_ether[i].state == UUID_ETHER_UNIQUE &&
	    bcmp(addr, uuid_ether[i].node, UUID_NODE_LEN))
		i++;
	if (i == UUID_NETHER || uuid_ether[i].state != UUID_ETHER_UNIQUE) {
		//mtx_unlock(&uuid_mutex);
		return (ENOENT);
	}

	// Remove it by shifting higher index entries down. 
	while (i < UUID_NETHER - 1 && uuid_ether[i].state != UUID_ETHER_EMPTY) {
		uuid_ether[i] = uuid_ether[i + 1];
		i++;
	}
	if (uuid_ether[i].state != UUID_ETHER_EMPTY) {
		uuid_ether[i].state = UUID_ETHER_EMPTY;
		bzero(uuid_ether[i].node, UUID_NODE_LEN);
	}
	//mtx_unlock(&uuid_mutex);
	return (0);
}


*/

int ICACHE_FLASH_ATTR
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

int ICACHE_FLASH_ATTR
printf_uuid(struct uuid *uuid)
{
	char buf[38];

	snprintf_uuid(buf, sizeof(buf), uuid);
	return (printf("%s", buf));
}

int ICACHE_FLASH_ATTR
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

void ICACHE_FLASH_ATTR
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

void ICACHE_FLASH_ATTR
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

void ICACHE_FLASH_ATTR
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

void ICACHE_FLASH_ATTR
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
// 
// int ICACHE_FLASH_ATTR
// parse_uuid(const char *str, struct uuid *uuid)
// {
// 	u_int c[11];
// 	int n;
// 
// 	/* An empty string represents a nil UUID. */
// 	if (*str == '\0') {
// 		bzero(uuid, sizeof(*uuid));
// 		return (0);
// 	}
// 
// 	/* The UUID string representation has a fixed length. */
// 	if (strlen(str) != 36)
// 		return (EINVAL);
// 
// 	/*
// 	 * We only work with "new" UUIDs. New UUIDs have the form:
// 	 *      01234567-89ab-cdef-0123-456789abcdef
// 	 * The so called "old" UUIDs, which we don't support, have the form:
// 	 *      0123456789ab.cd.ef.01.23.45.67.89.ab
// 	 */
// 	if (str[8] != '-')
// 		return (EINVAL);
// 
// 	n = sscanf(str, "%8x-%4x-%4x-%2x%2x-%2x%2x%2x%2x%2x%2x", c + 0, c + 1,
// 	    c + 2, c + 3, c + 4, c + 5, c + 6, c + 7, c + 8, c + 9, c + 10);
// 	/* Make sure we have all conversions. */
// 	if (n != 11)
// 		return (EINVAL);
// 
// 	/* Successful scan. Build the UUID. */
// 	uuid->time_low = c[0];
// 	uuid->time_mid = c[1];
// 	uuid->time_hi_and_version = c[2];
// 	uuid->clock_seq_hi_and_reserved = c[3];
// 	uuid->clock_seq_low = c[4];
// 	for (n = 0; n < 6; n++)
// 		uuid->node[n] = c[n + 5];
// 
// 	/* Check semantics... */
// 	return (((c[3] & 0x80) != 0x00 &&		/* variant 0? */
// 	    (c[3] & 0xc0) != 0x80 &&			/* variant 1? */
// 	    (c[3] & 0xe0) != 0xc0) ? EINVAL : 0);	/* variant 2? */
// }
