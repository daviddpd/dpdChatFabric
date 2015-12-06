#ifndef DPDCHATPACKETSTRUCT_H
#define DPDCHATPACKETSTRUCT_H

#define _CHATPACKET_ENCRYPTED 1
#define _CHATPACKET_CLEARTEXT 0

#define HOSTNAME_MAX_LENGTH 32

#include "__attribute__.h"
#include "uuid_wrapper.h"
#include "util.h"

extern int _GLOBAL_DEBUG;

#ifdef IOS_APP
#include "printf.h"
#endif


#ifdef ESP8266
#include "espconn.h"

#include "endian.h"
#include "mem.h"
#define socklen_t int
#define malloc os_malloc
#define calloc(len,size) calloc_wrapper(len,size);
#define free os_free
#define arc4random_uniform(x) os_random() % x


#else
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h> // htonl, ntohl
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
	char hwaddr[6];
	char * hostname;
	int status;
} hostmeta_t;

typedef struct  {
	uuid_cp u0;
	uuid_cp u1;
} uuid_tuple;

typedef struct {
	uint32_t control; // index of the control to act on.
	uint32_t type; // boolean(rw), dimmer(rw), gauge(ro), data(rw)
	int32_t value; // 1/0 ; 0~2^32; 0~2^32; length of data
	int32_t rangeLow;
	int32_t rangeHigh;
	int32_t gpio;
	uint32_t labelLength;
	char *label;
	unsigned char *data; 
//	void (*callback)(void *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd);	
} ESP_WORD_ALIGN cfControl;

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
	CMD_APP_LIST_ACK,	
	CMD_CONFIG_GET,
	CMD_CONFIG_SET,
	CMD_CONFIG_MESSAGE,
	CMD_CONFIG_DELIGATE,
	CMD_CONFIG_PAIR,
	CMD_SEND_REPLY_FALSE,
	CMD_SEND_REPLY_TRUE,
	CMD_PACKET_DECRYPT_FAILED,
	CMD_FAIL,

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

	 cptag_payloadRandomPaddingLength, // 15 / e
	 cptag_payloadRandomPaddingHigh,  // 16 / 0f
	 cptag_payloadRandomPaddingLow,	 // 17

	 cptag_encryptedPayload,

	 cptag_payload,

	 cptag_publickeyId,
	 cptag_publickey,

	 cptag_configLength,
	 cptag_config,


	 cptag_mac,
	 cptag_serial,
	 cptag_action,
	 cptag_action_type,
	 cptag_action_value,
	 cptag_action_control,
	 cptag_action_length,
	 cptag_action_data,
	 
	 cptag_app_controls,
	 cptag_app_control_i,
	 cptag_app_control_type,
	 cptag_app_control_value,
	 cptag_app_control_rangeLow,
	 cptag_app_control_rangeHigh,
	 cptag_app_control_labelLength,
	 cptag_app_control_label,
	 cptag_app_control_data,
	 
	 
	 cptag_ENDTAG,
	 	 	 
	 cptag_cmd = 0xFF,
	 	
} ESP_WORD_ALIGN;

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
	cftag_serial		= 10, // 1+4
	cftag_hasPairs		= 11, // 1+4

	cftag_debug, // 1+4
	cftag_port, // 1+4

	cftag_hostname, //  1+33 - hostname limit is 32char+null

	cftag_wifi_ap, // 1+1, wifi_ap_switch 
	cftag_wifi_ap_ssid, // 1+33, wifi_ap_ssid
	cftag_wifi_ap_passwd, // 1+65, wifi_ap_ssid

	cftag_wifi_ap_dhcps, // 1+1, wifi_ap_dhcps - server on AP.
	cftag_dhcps_range_low, // 1+4
	cftag_dhcps_range_high, // 1+4
	
	cftag_wifi_sta, // 1+1, wifi_sta_switch - be a station/client
	cftag_wifi_sta_ssid, // 1+33, wifi_ap_ssid
	cftag_wifi_sta_passwd, // 1+65, wifi_ap_ssid
	cftag_wifi_sta_dhcpc, // 1+1, dhcp_client_switch - server on AP.

	cftag_ap_ipv4, // 1+4
	cftag_ap_ipv4netmask, // 1+4
	cftag_ap_ipv4gw, // 1+4 wifi_sta_router
	cftag_ap_ipv4ns1, // 1+4
	cftag_ap_ipv4ns2, // 1+4

	cftag_ap_ipv6, // 1+16
	cftag_ap_ipv6netmask, // 1+16
	cftag_ap_ipv6gw, // 1+16
	cftag_ap_ipv6ns1, // 1+16
	cftag_ap_ipv6ns2, // 1+16

	cftag_sta_ipv4, // 1+4
	cftag_sta_ipv4netmask, // 1+4
	cftag_sta_ipv4gw, // 1+4 wifi_sta_router
	cftag_sta_ipv4ns1, // 1+4
	cftag_sta_ipv4ns2, // 1+4
	cftag_sta_ntpv4, // 1+4

	cftag_sta_ipv6, // 1+16
	cftag_sta_ipv6netmask, // 1+16
	cftag_sta_ipv6gw, // 1+16
	cftag_sta_ipv6ns1, // 1+16
	cftag_sta_ipv6ns2, // 1+16
	cftag_sta_ntpv6, // 1+4


	cftag_mode, // 1+4

	cftag_header		= 0xA5, // 165
	cftag_configLength	= 0xF0, // 240
	cftag_pairLength	= 0xF1, // 241
	cftag_pairs	= 0xF2, // 241

} ESP_WORD_ALIGN;

enum chatPacketActions {
	ACTION_NULL = 0,
	ACTION_GET,
	ACTION_SET,	
	ACTION_READ, // send
	ACTION_APP_LIST,
} ESP_WORD_ALIGN;

enum chatPacketActionsType {
	ACTION_TYPE_NULL = 0,
	ACTION_TYPE_BOOLEAN,
	ACTION_TYPE_DIMMER,
	ACTION_TYPE_GAUGE,
    ACTION_TYPE_DATA,
	ACTION_TYPE_DATA_UTF8,	
	ACTION_TYPE_DATA_BIN,
} ESP_WORD_ALIGN;


typedef struct {
	uint32_t action; // get/set/value
	uint32_t action_control; // index of the control to act on.
	uint32_t action_type; // boolean(rw), dimmer(rw), gauge(ro), data(rw)
	int32_t action_value; // 1/0 ; 0~2^32; 0~2^32; undef
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
	
	cfControl *controlers;
	int numOfControllers;
	
	
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
	int32_t action_value; // 1/0 ; 0~2^32; 0~2^32; undef
	uint32_t action_length; 
	unsigned char *action_data;
	
	
	
} ESP_WORD_ALIGN chatPacket;


typedef struct {
	int socket;
#ifdef ESP8266
	struct espconn tcpconn;
	struct espconn udpconn;
	struct espconn *conn;
	esp_tcp esptcp;
#else
	struct sockaddr_in sockaddr;
#endif
	char *ip;
	int type; // SOCK_STREAM SOCK_DGRAM SOCK_RAW SOCK_RDM SOCK_SEQPACKET
	int acceptedSocket;
	int bind;
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
	int32_t action_value; // 1/0 ; 0~2^32; 0~2^32; undef
	uint32_t action_length; 
	unsigned char *action_data;
} ESP_WORD_ALIGN msgbuffer;


typedef struct  {
	char *configfile;
	char *newconfigfile;
	char *pairfile;
	char *ip;
	unsigned char *msg;
	cfControl *controlers;
	int numOfControllers;
	void (*callback)(void *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd);	
	
	int port;
	int type; // SOCK_STREAM SOCK_DGRAM SOCK_RAW SOCK_RDM SOCK_SEQPACKET
	
	int hasPairs;
	uuid_tuple uuid;
//	uuid_tuple to;
	
	int debug;
	int writeconfig;
	int mode;
	
	char hostname[33];

    // WiFi Station IP Configuation
	uint32_t sta_ipv4; // 1+4
	uint32_t sta_ipv4netmask; // 1+4
	uint32_t sta_ipv4gw; // 1+4
	uint32_t sta_ipv4ns1; // 1+4
	uint32_t sta_ipv4ns2; // 1+4	
	uint32_t ntpv4; // 1+4	

	unsigned char sta_ipv6[16]; // 1+16
	unsigned char sta_ipv6netmask[16]; // 1+16
	unsigned char sta_ipv6gw[16]; // 1+16
	unsigned char sta_ipv6ns1[16]; // 1+16
	unsigned char sta_ipv6ns2[16]; // 1+16	
	unsigned char ntpv6[16]; // 1+16


    // WiFi Access Point IP Configuation
	uint32_t ap_ipv4; // 1+4
	uint32_t ap_ipv4netmask; // 1+4
	uint32_t ap_ipv4gw; // 1+4
	uint32_t ap_ipv4ns1; // 1+4
	uint32_t ap_ipv4ns2; // 1+4	

	unsigned char ap_ipv6[16]; // 1+16
	unsigned char ap_ipv6netmask[16]; // 1+16
	unsigned char ap_ipv6gw[16]; // 1+16
	unsigned char ap_ipv6ns1[16]; // 1+16
	unsigned char ap_ipv6ns2[16]; // 1+16


	int wifi_ap_switch; 
	char wifi_ap_ssid[33];
    char wifi_ap_passwd[65];

    // Wifi Access Point - DHCP Server
	int wifi_ap_dhcps_switch;
	uint32_t dhcps_range_low;
	uint32_t dhcps_range_high;
        
    // Wifi Station/Client
	int wifi_sta_switch;
	char wifi_sta_ssid[33];
	char wifi_sta_passwd[65];

    // WiFi Station DHCP Client
	int dhcp_client_switch;
	
	unsigned char publickey[crypto_box_PUBLICKEYBYTES];
	unsigned char privatekey[crypto_box_SECRETKEYBYTES];

	
} ESP_WORD_ALIGN chatFabricConfig;


#endif
