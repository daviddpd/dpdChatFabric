
#ifndef DPDCHATFABRIC_H
#define DPDCHATFABRIC_H


#include <arpa/inet.h>
#include <errno.h>

#include <netinet/in.h> // htonl, ntohl
#include <stdio.h> // printf
#include <stdlib.h> // exit
#include <string.h> //memcpy
#include <strings.h> // bzero
#include <sys/errno.h>
#include <sys/types.h> // kqueue / kevent 
#include <sys/event.h> // kqueue / kevent 
#include <sys/socket.h>
#include <sys/time.h>// kqueue / kevent 
#include <sys/wait.h> // fork and wait
#include <unistd.h> // fork and wait, getpid
#include <uuid.h>   // uuid

#include <limits.h>
     
#include <getopt.h>

#include <ucl.h>
#include <sodium.h>

#define _UUID0 "00000000-0000-0000-0000-000000000000"

#define _CHATPACKET_ENCRYPTED 1
#define _CHATPACKET_CLEARTEXT 0



enum chatPacketTags  {	
	 cptag_envelopeKey			= 0,
	 cptag_envelopeNonce		= 1,
	 cptag_envelopeLength		= 2,

	 cptag_encryptedEnvelope	= 130,  // set bit 7 high 

	 cptag_envelopeRandomPaddingLength	= 3,
	 cptag_envelopeRandomPaddingHigh	= 4,
	 cptag_envelopeRandomPaddingLow	= 132,  // set bit 7 high 	

	 cptag_to0					= 5,
	 cptag_to1					= 6,
	 cptag_from0				= 7,
	 cptag_from1				= 8,

	 cptag_msgid				= 9,
	 cptag_flags				= 10,

	 cptag_payloadKey			= 11,
	 cptag_payloadNonce		= 12,
	 cptag_payloadLength		= 13,

	 cptag_payloadRandomPaddingLength	= 14,
	 cptag_payloadRandomPaddingHigh	= 15,
	 cptag_payloadRandomPaddingLow		= 143,  // set bit 7 high 	
	
	 cptag_encryptedPayload	= 144,  // set bit 7 high 	
	 cptag_payload				= 16,
	
};


typedef struct  {
	uuid_t u0;
	uuid_t u1;
} uuid_tuple;

typedef struct {
	size_t length;
	unsigned char *msg;
} msgbuffer;

typedef struct  {
	uuid_t envelopeKey;
	uint64_t envelopeNonce;
	uint32_t envelopeLength;

	unsigned char envelopeRandomPaddingLength;
	unsigned char envelopeRandomPadding[16];

	uuid_t to0;
	uuid_t to1;
	uuid_t from0;
	uuid_t from1;

	uint32_t msgid;
	uint32_t flags;

	uuid_t payloadKey;
	uint64_t payloadNonce;
	uint32_t payloadLength;

	unsigned char payloadRandomPaddingLength;
	unsigned char payloadRandomPadding[16];
	
	unsigned char *payload;
	
} chatPacket;


typedef struct  {
	char *configfile;
	char *dbfile;
	char *ip;
	uuid_t uuid0;
	uuid_t uuid1;
	unsigned char *payloadkeys_public_str;
	unsigned char *payloadkeys_private_str;

	unsigned char payloadkeys_public[crypto_box_PUBLICKEYBYTES];
	unsigned char payloadkeys_private[crypto_box_SECRETKEYBYTES];

	unsigned char *envelopekeys_public_str;
	unsigned char *envelopekeys_private_str;

	unsigned char envelopekeys_public[crypto_box_PUBLICKEYBYTES];
	unsigned char envelopekeys_private[crypto_box_SECRETKEYBYTES];

	unsigned char *peerkeys_envelope_public_str;
	unsigned char *peerkeys_payload_public_str;
	
	unsigned char peerkeys_envelope_public[crypto_box_PUBLICKEYBYTES];
	unsigned char peerkeys_payload_public[crypto_box_PUBLICKEYBYTES];


	uint64_t envelopeNonce;
	uint64_t payloadNonce;
	uint32_t msgid;
	
} chatFabricConfig;


void print_bin2hex(unsigned char * x, int len);

void 
chatFabric_hexprint ( unsigned char *str, uint32_t len );

void
chatFabric_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght );

void
chatFabric_usage(char *p);

void
chatFabric_args(int argc, char**argv, chatFabricConfig *config);

void
chatFabric_configParse(chatFabricConfig *config);


chatPacket*
chatPacket_init (chatFabricConfig *config, uuid_tuple *to, unsigned char *payload, uint32_t len, uint32_t flags);

void
chatPacket_delete (chatPacket* cp);

void
chatPacket_encode (chatPacket *cp, chatFabricConfig *, msgbuffer *ob, int encrypted);

void 
chatPacket_decode (chatPacket *cp, unsigned char *b, ssize_t len, chatFabricConfig *config);

void 
chatPacket_print (chatPacket *cp);



#endif