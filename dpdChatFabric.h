
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

#include <sodium.h>
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

enum chatPacketCommands 
stateMachine (chatFabricConfig *config, chatPacket *cp, chatFabricPairing *pair, chatPacket *reply);



#endif

