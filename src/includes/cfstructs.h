#ifndef DPDCHATPACKETSTRUCT_H
#define DPDCHATPACKETSTRUCT_H

#define _CHATPACKET_ENCRYPTED 1
#define _CHATPACKET_CLEARTEXT 0


#ifdef ESP8266
#include "endian.h"
#include "mem.h"
#define CHATFABRIC_DEBUG(d, msg) if (d)  os_printf("[DEBUG][%5d:%s:%s:%d] %s\n", heapLast - system_get_free_heap_size(), __FILE__, __FUNCTION__, __LINE__, msg );

#define CHATFABRIC_DEBUG_FMT(d, ...) if (d) os_printf( __VA_ARGS__ )
#define socklen_t int
#define calloc os_zalloc
#define malloc os_malloc
#define free os_free
#define arc4random_uniform(x) os_random() % x


#else
#define CHATFABRIC_DEBUG(d, msg) if (d) fprintf(stderr, "[DEBUG][%s:%s:%d] %s\n", __FILE__, __FUNCTION__, __LINE__, msg )
#define CHATFABRIC_DEBUG_FMT(d, ...) if (d) fprintf(stderr, __VA_ARGS__ )
#endif

#ifdef HAVE_LOCAL_CRYPTO
#ifndef crypto_box_PUBLICKEYBYTES
#define crypto_box_PUBLICKEYBYTES 32U
#define crypto_secretbox_MACBYTES 16U
#define crypto_secretbox_NONCEBYTES 8U
#define crypto_box_SEALBYTES 16U
#define crypto_box_SECRETKEYBYTES 32U
#endif
#endif



typedef struct  {
	uuid_t u0;
	uuid_t u1;
} uuid_tuple;

enum chatPacketDirection {
	NONE,
	IN,
	OUT,
} ESP_WORD_ALIGN;

enum chatPacketTagData {
	CP_NONE,
	CP_NOOP,

	CP_INT32,
	CP_INT8,

	CP_DATA8,
	CP_UUID,
	
} ESP_WORD_ALIGN;


enum chatFabricErrors {
	ERROR_OK = 0,
	ERROR_SOCKET = -1,
	ERROR_CHATPACKET_DECODING = -2,
	ERROR_INVAILD_DEST = -3,
	
} ESP_WORD_ALIGN;

enum chatPacketStates  {	
	STATE_UNCONFIGURED,
	STATE_PUBLICKEY_SETUP,

	STATE_PAIRING_SETUP,
	STATE_NONCE_SETUP,

	STATE_CONFIGURED,
	STATE_CONFIGURED_SYN,

	STATE_CONFIGURED_SYNACK,
	STATE_PAIRED,

} ESP_WORD_ALIGN;


enum chatPacketPacketTypes {
	COMMAND,
	PUBLICKEY,
	NONCE,
	DATA,
} ESP_WORD_ALIGN;


enum chatPacketCommands {	
	CMD_INVAILD_CMD,
	CMD_HELLO,
	CMD_HELLO_ACK,
	CMD_PAIR_REQUEST,
	CMD_PAIR_REQUEST_ACK,
	CMD_PAIR_REQUEST_RESET,
	CMD_NONCE_REQUEST,
	CMD_NONCE_SEND,
	CMD_NONCE_ACK,
	CMD_NONCE_RESET,
	CMD_PUBLICKEY_REQUEST,
	CMD_PUBLICKEY_SEND,
	CMD_PUBLICKEY_ACK,
	CMD_PUBLICKEY_RESET,
	CMD_VERIFY_SYN,
	CMD_VERIFY_SYNACK,
	CMD_VERIFY_ACK,
	CMD_APP_MESSAGE,
	CMD_APP_MESSAGE_ACK,
	CMD_APP_REGISTER,
	CMD_APP_LIST,
	CMD_CONFIG_MESSAGE,
	CMD_CONFIG_DELIGATE,
	CMD_CONFIG_PAIR,
	CMD_SEND_REPLY_FALSE,
	CMD_SEND_REPLY_TRUE,

} ESP_WORD_ALIGN;


enum chatPacketTags  {	
	 cptag_NOOP, 						// 0

	 cptag_nonceLength,	 				// 1
	 cptag_nonce,						// 2

	 cptag_envelopeLength,				// 3
	 cptag_encryptedEnvelope,			// 4
	 cptag_envelope,					// 5

	 cptag_envelopeRandomPaddingLength, // 6
	 cptag_envelopeRandomPaddingHigh,   // 7
	 cptag_envelopeRandomPaddingLow,    // 8

	 cptag_to0,							// 9
	 cptag_to1,							// 10 / a
	 cptag_from0,						// 11 / b
	 cptag_from1,						// 12 / c
	 cptag_flags,						// 13 / d

	 cptag_payloadLength,				// 14 / d

	 cptag_payloadRandomPaddingLength,
	 cptag_payloadRandomPaddingHigh,
	 cptag_payloadRandomPaddingLow,	

	 cptag_encryptedPayload,

	 cptag_payload,

	 cptag_publickeyId,
	 cptag_publickey,

	 cptag_mac,
	 cptag_serial,
	 cptag_action,
	 cptag_action_type,
	 cptag_action_value,
	 cptag_action_control,
	 cptag_action_length,
	 cptag_action_data,
	 	 	 
	 cptag_cmd = 0xFF,
	 	
} ESP_WORD_ALIGN;


enum chatPacketActions {
	ACTION_NULL = 0,
	ACTION_GET,
	ACTION_SET,	
	ACTION_READ, // send	
} ESP_WORD_ALIGN;

enum chatPacketActionsType {
	ACTION_TYPE_NULL = 0,
	ACTION_TYPE_BOOLEAN,
	ACTION_TYPE_DIMMER,
	ACTION_TYPE_GAUGE,
	ACTION_TYPE_DATA,	
} ESP_WORD_ALIGN;


typedef struct {
	uint32_t action; // get/set/value
	uint32_t action_control; // index of the control to act on.
	uint32_t action_type; // boolean(rw), dimmer(rw), gauge(ro), data(rw)
	uint32_t action_value; // 1/0 ; 0~2^32; 0~2^32; undef
	uint32_t action_length; 
	unsigned char *action_data;
	
} ESP_WORD_ALIGN chatFabricAction;


typedef struct  {
	uint32_t cmd;
	uint32_t flags;
	uint32_t payloadLength;
#ifdef ESP8266
	int cpindex;
#endif
	uint32_t serial;	

	uint32_t wasEncrypted;
	
	unsigned char nonce[crypto_secretbox_NONCEBYTES];
	uint32_t envelopeLength;

	unsigned char envelopeRandomPaddingLength;
	unsigned char envelopeRandomPadding[16];

	uuid_tuple to;
	uuid_tuple from;


	unsigned char publickey[crypto_box_PUBLICKEYBYTES];

	unsigned char payloadRandomPaddingLength;
	unsigned char payloadRandomPadding[16];
	
	unsigned char *payload;
	
	uint32_t action; // get/set/value
	uint32_t action_control; // index of the control to act on.
	uint32_t action_type; // boolean(rw), dimmer(rw), gauge(ro), data(rw)
	uint32_t action_value; // 1/0 ; 0~2^32; 0~2^32; undef
	uint32_t action_length; 
	unsigned char *action_data;
	
	
	
} ESP_WORD_ALIGN chatPacket;


typedef struct {
	int socket;
#ifdef ESP8266
	struct espconn conn;
#else
	struct sockaddr_in sockaddr;
#endif
	char *ip;	
} ESP_WORD_ALIGN chatFabricConnection;

typedef struct  {
	uint32_t hasPublicKey;	
	uint32_t hasNonce;	
	uint32_t serial;	

	enum chatPacketStates ESP_WORD_ALIGN state;
	
	uuid_tuple uuid;
	unsigned char publickey[crypto_box_PUBLICKEYBYTES];
	unsigned char sharedkey[crypto_box_PUBLICKEYBYTES];
	
	unsigned char nullnonce[crypto_secretbox_NONCEBYTES];

	unsigned char nonce[crypto_secretbox_NONCEBYTES];
	unsigned char mynonce[crypto_secretbox_NONCEBYTES];

} ESP_WORD_ALIGN chatFabricPairing;


typedef struct {
	int length;
	unsigned char *msg;
	uint32_t action; // get/set/value
	uint32_t action_control; // index of the control to act on.
	uint32_t action_type; // boolean(rw), dimmer(rw), gauge(ro), data(rw)
	uint32_t action_value; // 1/0 ; 0~2^32; 0~2^32; undef
	uint32_t action_length; 
	unsigned char *action_data;
} ESP_WORD_ALIGN msgbuffer;


typedef struct  {
	char *configfile;
	char *newconfigfile;
	char *pairfile;
	char *ip;
	unsigned char *msg;
	void (*callback)(void *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd);	
	
	int port;
	int hasPairs;
	uuid_tuple uuid;
	uuid_tuple to;
	
	int debug;
	int writeconfig;
	
	unsigned char publickey[crypto_box_PUBLICKEYBYTES];
	unsigned char privatekey[crypto_box_SECRETKEYBYTES];

	
} ESP_WORD_ALIGN chatFabricConfig;
#endif
