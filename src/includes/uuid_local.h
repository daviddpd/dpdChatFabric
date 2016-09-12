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
 *
 * $FreeBSD: stable/10/sys/sys/uuid.h 253590 2013-07-24 04:24:21Z marcel $
 */
#ifdef ESP8266
//#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#ifndef CP_ICACHE_FLASH_ATTR
#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#endif
#else
#define CP_ICACHE_FLASH_ATTR __attribute__(())
#endif

#ifndef _SYS_UUID_H_
#define	_SYS_UUID_H_

#include <c_types.h>
#include <sys/cdefs.h>
#include <sys/time.h>
#include <driver/errno.h>
#include <osapi.h>
#include <endian.h>

/* Length of a node address (an IEEE 802 address). */
#define	_UUID_NODE_LEN		6

/*
 * See also:
 *      http://www.opengroup.org/dce/info/draft-leach-uuids-guids-01.txt
 *      http://www.opengroup.org/onlinepubs/009629399/apdxa.htm
 *
 * A DCE 1.1 compatible source representation of UUIDs.
 */

struct bintime {
	time_t	sec;
	uint64_t frac;
};
void	bintime(struct bintime *bt);
 
 
struct uuid {
	uint32_t	time_low;
	uint16_t	time_mid;
	uint16_t	time_hi_and_version;
	uint8_t		clock_seq_hi_and_reserved;
	uint8_t		clock_seq_low;
	uint8_t	node[_UUID_NODE_LEN];
};

extern int os_printf_plus(const char * format, ...);

#define	UUID_NODE_LEN	_UUID_NODE_LEN

#define bcopy(src,dst,len) os_memcpy(dst,src,len)
#define bzero(src,len) os_bzero(src,len)
#define strlen os_strlen
// #define sscanf os_sscanf
#define sscanf nsscanf
#define arc4random os_random 

#define printf(...) os_printf( __VA_ARGS__ )
#define sprintf(...) os_sprintf( __VA_ARGS__ )
#define sbuf_printf(...) os_sprintf( __VA_ARGS__ )

#define bswap16 __builtin_bswap16


// #define snprintf(str,size,fmt, __VA_ARGS__) os_sprintf(str,fmt,__VA_ARGS__)

struct sbuf;

struct uuid *kern_uuidgen(struct uuid *, size_t);

int uuid_ether_add(const uint8_t *);
int uuid_ether_del(const uint8_t *);

int snprintf_uuid(char *, size_t, struct uuid *);
int printf_uuid(struct uuid *);
int sbuf_printf_uuid(struct sbuf *, struct uuid *);
//int parse_uuid(const char *, struct uuid *);

void be_uuid_dec(void const *buf, struct uuid *uuid);
void be_uuid_enc(void *buf, struct uuid const *uuid);
void le_uuid_dec(void const *buf, struct uuid *uuid);
void le_uuid_enc(void *buf, struct uuid const *uuid);


/* XXX namespace pollution? */
typedef struct uuid uuid_cp;

__BEGIN_DECLS
int	uuidgen(struct uuid *, int);
__END_DECLS


#endif /* _SYS_UUID_H_ */
/*-
 * Copyright (c) 2002,2005 Marcel Moolenaar
 * Copyright (c) 2002 Hiten Mahesh Pandya
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD: stable/10/include/uuid.h 183057 2008-09-15 23:47:19Z emax $
 */

#ifndef _UUID_H_
#define	_UUID_H_


/*
 * This implementation mostly conforms to the DCE 1.1 specification.
 * See Also:
 *	uuidgen(1), uuidgen(2), uuid(3)
 */

/* Status codes returned by the functions. */
#define	uuid_s_ok			0
#define	uuid_s_bad_version		1
#define	uuid_s_invalid_string_uuid	2
#define	uuid_s_no_memory		3

__BEGIN_DECLS
int32_t	uuid_compare(const uuid_cp *, const uuid_cp *, uint32_t *);
void	uuid_create(uuid_cp *, uint32_t *);
void	uuid_create_nil(uuid_cp *, uint32_t *);
int32_t	uuid_equal(const uuid_cp *, const uuid_cp *, uint32_t *);
void	uuid_from_string(const char *, uuid_cp *, uint32_t *);
uint16_t uuid_hash(const uuid_cp *, uint32_t *);
int32_t	uuid_is_nil(const uuid_cp *, uint32_t *);
void	uuid_to_string(const uuid_cp *, char **, uint32_t *);
void	uuid_enc_le(void *, const uuid_cp *);
void	uuid_dec_le(const void *, uuid_cp *);
void	uuid_enc_be(void *, const uuid_cp *);
void	uuid_dec_be(const void *, uuid_cp *);
__END_DECLS

#endif	/* _UUID_H_ */
