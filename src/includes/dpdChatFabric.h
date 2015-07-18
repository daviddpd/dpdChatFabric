
#ifndef DPDCHATFABRIC_H
#define DPDCHATFABRIC_H

#ifdef ESP8266
#define ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#else
#define ICACHE_FLASH_ATTR __attribute__(())
#endif


#include <arpa/inet.h>
#include <netinet/in.h> // htonl, ntohl
#include <limits.h>
#include <errno.h>
#include <stdio.h> // printf


#include <stdlib.h> // exit
#include <string.h> //memcpy
#include <strings.h> // bzero
#include <sys/errno.h>
#include <sys/types.h> // kqueue / kevent 
#include <sys/event.h> // kqueue / kevent 
#include <sys/socket.h>
#include <sys/time.h> // kqueue / kevent 
#include <sys/wait.h> // fork and wait
#include <unistd.h> // fork and wait, getpid

     
#include <getopt.h>
#include <uuid.h>   // uuid

#ifdef HAVE_SODIUM
#include <sodium.h>
#endif 

#ifdef HAVE_LOCAL_CRYPTO
#include <salsa20.h>
#include <poly1305-donna.h>
#include <chacha20_simple.h>

#define crypto_box_PUBLICKEYBYTES 32U
#define crypto_secretbox_MACBYTES 16U
#define crypto_secretbox_NONCEBYTES 8U
#define crypto_box_SEALBYTES 16U
#define crypto_box_SECRETKEYBYTES 32U


void curve25519_donna(unsigned char *output, const unsigned char *a, const unsigned char *b);

#endif


#include "cfstructs.h"
#include "dpdChatPacket.h"

enum chatFabricConfigTags  {	

	 cftag_publickey	= 1, // 1+crypto_box_PUBLICKEYBYTES
	 cftag_privatekey	= 2, // 1+crypto_box_SECRETKEYBYTES

	 cftag_nonce		= 3, // 1+crypto_secretbox_NONCEBYTES
	 cftag_mynonce		= 4, // 1+crypto_secretbox_NONCEBYTES

	 cftag_uuid0		= 5, // 1+16
	 cftag_uuid1		= 6, // 1+16

	 cftag_hasPublicKey	= 7, // 1+1
	 cftag_hasNonce		= 8, // 1+1
	 cftag_state		= 9, // 1+1
	 
	
};



const char * stateLookup (enum chatPacketStates state);
const char * cmdLookup (enum chatPacketCommands cmd);

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
void
chatFabric_pairConfig(chatFabricConfig *config, chatFabricPairing *pair, int write );

enum chatFabricErrors 
chatFabric_controller(chatFabricConnection *c, chatFabricPairing *pair, chatFabricConfig *config,  msgbuffer *b);


void
chatFabric_configParse(chatFabricConfig *config);

void
chatFabric_consetup( chatFabricConnection *c,  char *ip, int port, int doBind );

enum chatFabricErrors 
chatFabric_device(chatFabricConnection *c, chatFabricPairing *pair, chatFabricConfig *config,  msgbuffer *b);


void
stateMachine (chatFabricConfig *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd, enum chatFabricErrors *e);



#endif
