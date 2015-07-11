
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

enum chatPacketDirection {
	NONE,
	IN,
	OUT,
};

enum chatPacketStates  {	
	UNCONFIGURED,
	PUBLICKEY_SETUP,
	PAIRING_SETUP,
	NONCE_SETUP,
	CONFIGURED,
	CONFIGURED_SYN,
	CONFIGURED_SYNACK,
	PAIRED,
};


enum chatPacketPacketTypes {
	COMMAND,
	PUBLICKEY,
	NONCE,
	DATA,
};


enum chatPacketCommands {	
	INVAILD_CMD,
	HELLO,
	HELLO_ACK,
	PAIR_REQUEST,
	PAIR_REQUEST_ACK,
	PAIR_REQUEST_RESET,
	NONCE_REQUEST,
	NONCE_SEND,
	NONCE_ACK,
	NONCE_RESET,
	PUBLICKEY_REQUEST,
	PUBLICKEY_SEND,
	PUBLICKEY_ACK,
	PUBLICKEY_RESET,
	VERIFY_SYN,
	VERIFY_SYNACK,
	VERIFY_ACK,	
	APP_MESSAGE,
	APP_REGISTER,
	APP_LIST,
	CONFIG_MESSAGE,
	CONFIG_DELIGATE,
	CONFIG_PAIR,
	SEND_REPLY_FALSE,
	SEND_REPLY_TRUE,

};


enum chatPacketTags  {	
	 cptag_nonce		= 1,
	 cptag_envelopeLength		= 2,

	 cptag_encryptedEnvelope	= 130,  // set bit 7 high 

	 cptag_envelopeRandomPaddingLength	= 3,
	 cptag_envelopeRandomPaddingHigh	= 4,
	 cptag_envelopeRandomPaddingLow	= 132,  // set bit 7 high 	

	 cptag_to0					= 5,
	 cptag_to1					= 6,
	 cptag_from0				= 7,
	 cptag_from1				= 8,

	 cptag_flags				= 10,

	 cptag_payloadLength		= 13,

	 cptag_payloadRandomPaddingLength	= 14,
	 cptag_payloadRandomPaddingHigh	= 15,
	 cptag_payloadRandomPaddingLow		= 143,  // set bit 7 high 	
	
	 cptag_encryptedPayload	= 144,  // set bit 7 high 	
	 cptag_payload				= 16,
	 cptag_publickey			= 17,
	 
	 cptag_cmd				= 170 // 1010 1010
	 
	
};



typedef struct  {
	uuid_t u0;
	uuid_t u1;
} uuid_tuple;

typedef struct  {
	enum chatPacketStates state;
	int hasPublicKey;	

} chatFabricState;

typedef struct  {
	uuid_tuple uuid;
	unsigned char publickey[crypto_box_PUBLICKEYBYTES];
	int hasPublicKey;	
	int hasNonce;	
	unsigned char nonce[crypto_secretbox_NONCEBYTES];
	unsigned char mynonce[crypto_secretbox_NONCEBYTES];

} chatFabricPairing;


typedef struct {
	size_t length;
	unsigned char *msg;
} msgbuffer;

typedef struct  {
	uint32_t cmd;
	uint32_t flags;

	unsigned char nonce[crypto_secretbox_NONCEBYTES];
	uint32_t envelopeLength;

	unsigned char envelopeRandomPaddingLength;
	unsigned char envelopeRandomPadding[16];

	uuid_t to0;
	uuid_t to1;
	uuid_t from0;
	uuid_t from1;

	uint32_t payloadLength;

	unsigned char publickey[crypto_box_PUBLICKEYBYTES];

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
	unsigned char *publickey_str;
	unsigned char *privatekey_str;

	unsigned char publickey[crypto_box_PUBLICKEYBYTES];
	unsigned char privatekey[crypto_box_SECRETKEYBYTES];

	unsigned char *publickey_str2;
	unsigned char *privatekey_str2;

	unsigned char publickey2[crypto_box_PUBLICKEYBYTES];
	unsigned char privatekey2[crypto_box_SECRETKEYBYTES];

	
} chatFabricConfig;

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

chatPacket*
chatPacket_init0 (void);

chatPacket*
chatPacket_init (chatFabricConfig *config, chatFabricPairing *pair, enum chatPacketCommands cmd, unsigned char *payload, uint32_t len, uint32_t flags);

void
chatPacket_delete (chatPacket* cp);

void
chatPacket_encode (chatPacket *cp, chatFabricConfig *config, chatFabricPairing *pair, msgbuffer *ob, int encrypted, enum chatPacketPacketTypes packetType);

int chatPacket_decode (chatPacket *cp,  chatFabricPairing *pair, unsigned char *b, const int len, chatFabricConfig *config);

void 
chatPacket_print (chatPacket *cp, enum chatPacketDirection d);

enum chatPacketCommands 
stateMachine (chatFabricConfig *config, chatFabricState *s, chatPacket *cp, chatFabricPairing *pair, chatPacket *reply);



#endif