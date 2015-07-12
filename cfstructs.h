#ifndef DPDCHATPACKETSTRUCT_H
#define DPDCHATPACKETSTRUCT_H

#define _CHATPACKET_ENCRYPTED 1
#define _CHATPACKET_CLEARTEXT 0

#define CHATFABRIC_DEBUG(d, msg) if (d) fprintf(stderr, "[DEBUG][%s:%s:%d] %s\n", __FILE__, __FUNCTION__, __LINE__, msg )
#define CHATFABRIC_DEBUG_FMT(d, ...) if (d) fprintf(stderr, __VA_ARGS__ )


typedef struct  {
	uuid_t u0;
	uuid_t u1;
} uuid_tuple;

enum chatPacketDirection {
	NONE,
	IN,
	OUT,
};


enum chatFabricErrors {
	ERROR_OK = 0,
	ERROR_SOCKET = -1,
	ERROR_CHATPACKET_DECODING = -2,
	ERROR_INVAILD_DEST = -3,
	
};

enum chatPacketStates  {	
	STATE_UNCONFIGURED,
	STATE_PUBLICKEY_SETUP,
	STATE_PAIRING_SETUP,
	STATE_NONCE_SETUP,
	STATE_CONFIGURED,
	STATE_CONFIGURED_SYN,
	STATE_CONFIGURED_SYNACK,
	STATE_PAIRED,
};


enum chatPacketPacketTypes {
	COMMAND,
	PUBLICKEY,
	NONCE,
	DATA,
};


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
	CMD_CMD_VERIFY_SYNACK,
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
	uint32_t cmd;
	uint32_t flags;

	uint32_t wasEncrypted;
	
	unsigned char nonce[crypto_secretbox_NONCEBYTES];
	uint32_t envelopeLength;

	unsigned char envelopeRandomPaddingLength;
	unsigned char envelopeRandomPadding[16];

	uuid_tuple to;
	uuid_tuple from;

	uint32_t payloadLength;

	unsigned char publickey[crypto_box_PUBLICKEYBYTES];

	unsigned char payloadRandomPaddingLength;
	unsigned char payloadRandomPadding[16];
	
	unsigned char *payload;
	
} chatPacket;


typedef struct {
	int socket;
	struct sockaddr_in sockaddr;
	char *ip;	
} chatFabricConnection;

typedef struct  {
	int hasPublicKey;	
	int hasNonce;	

	enum chatPacketStates state;
	
	uuid_tuple uuid;
	unsigned char publickey[crypto_box_PUBLICKEYBYTES];

	unsigned char nonce[crypto_secretbox_NONCEBYTES];
	unsigned char mynonce[crypto_secretbox_NONCEBYTES];

} chatFabricPairing;


typedef struct {
	int length;
	unsigned char *msg;
} msgbuffer;



typedef struct  {
	char *configfile;
	char *newconfigfile;
	char *pairfile;
	char *ip;
	unsigned char *msg;
	int port;
	uuid_tuple uuid;
	
	int debug;
	int writeconfig;
	
	unsigned char publickey[crypto_box_PUBLICKEYBYTES];
	unsigned char privatekey[crypto_box_SECRETKEYBYTES];

	
} chatFabricConfig;
#endif
