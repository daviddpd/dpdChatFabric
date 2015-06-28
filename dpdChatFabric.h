
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

/**
 ** field             offset ***********
    -----------------------------------------
*/

#define OFFSET_ENVELOPEKEY       0
#define OFFSET_ENVELOPENONCE     16
#define OFFSET_ENVELOPELENGTH    24

#define OFFSET_TO0               28
#define OFFSET_TO1               44
#define OFFSET_FROM0             60
#define OFFSET_FROM1             76

#define OFFSET_MSGID             92
#define OFFSET_FLAGS             96

#define OFFSET_PAYLOADKEY       100
#define OFFSET_PAYLOADNOCE      116

#define OFFSET_PAYLOADLENGTH    124
#define OFFSET_PAYLOAD          128


typedef struct  {
	uuid_t envelopeKey;				//   16 bytes:    0 -  15
	uint64_t envelopeNonce;			//    8 bytes:   16 -  23
	uint32_t envelopeLength;		//    4 bytes:   24 -  27

	uuid_t to0;						//   16 bytes:   28 -  43
	uuid_t to1;						//   16 bytes:   44 -  59
	uuid_t from0;					//   16 bytes:   60 -  75
	uuid_t from1;					//   16 bytes:   76 -  91

	uint32_t msgid;					//    4 bytes:   92 -  95
	uint32_t flags;					//    4 bytes:   96 -  99

	uuid_t payloadKey;				//   16 bytes:  100 - 115 
	uint64_t payloadNonce;			//    8 bytes:  116 - 123
	uint32_t payloadLength;			//    4 bytes:  124 - 127

									// 1400 - 128  = 1272 
									//              -   1 byte (random padding length)
									//              -  16 bytes (random padding)
									//              -  16 (AEAD) envelope
									//              -  16 (AEAD) payload
									

	char payload[1223];				// 1223 bytes:   128 - 1350
									//   16 bytes:  1350 - 1365 AEAD payload
									//   16 bytes:  1366 - 1381 AEAD envelope
									//      bytes:  1382 - 1397 random padding
									
									
									//    1 bytes 
									//             - high nibble, - length of head padding
									//             - low nibble, - length of tailling padding
									//   16 bytes - random padding
	
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
	
} chatFabricConfig;

void
chatFabric_hex2int_bytes (unsigned char *hex, uint32_t hexLength, unsigned char *dst, uint32_t dstLenght );

void
chatFabric_usage(char *p);

void
chatFabric_args(int argc, char**argv, chatFabricConfig *config);

void
chatFabric_configParse(chatFabricConfig *config);


uint32_t 
chatPacket_encode (chatPacket *cp, unsigned char *b, chatFabricConfig *);

void 
chatPacket_decode (chatPacket *cp, unsigned char *b, chatFabricConfig *);

void 
chatPacket_print (chatPacket *cp);



#endif