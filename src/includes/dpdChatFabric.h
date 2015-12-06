
#ifndef DPDCHATFABRIC_H
#define DPDCHATFABRIC_H

#ifdef ESP8266
//#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#ifndef CP_ICACHE_FLASH_ATTR
#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#endif
#define ESP_WORD_ALIGN __attribute__ ((aligned (4)))
#else
#define CP_ICACHE_FLASH_ATTR __attribute__(())
#define ESP_WORD_ALIGN __attribute__ (())
#endif


#ifdef ESP8266
#include "ets_sys.h"
#include "osapi.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"
#include "mem.h"
#include "arc4random_buf.h"

#else 
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
#endif 

#include "uuid_wrapper.h"

#ifdef HAVE_SODIUM
#include <sodium.h>
#endif 

#ifdef HAVE_LOCAL_CRYPTO
#include "salsa20.h"
#include "poly1305-donna.h"
#include "chacha20_simple.h"

#define crypto_box_PUBLICKEYBYTES 32U
#define crypto_secretbox_MACBYTES 16U
#define crypto_secretbox_NONCEBYTES 8U
#define crypto_box_SEALBYTES 16U
#define crypto_box_SECRETKEYBYTES 32U


void curve25519_donna(unsigned char *output, const unsigned char *a, const unsigned char *b);

#endif


#include "cfstructs.h"
#include "dpdChatPacket.h"




const char * stateLookup (enum chatPacketStates state);
const char * cmdLookup (enum chatPacketCommands cmd);

void chatFabric_pairConfig(chatFabricConfig *config, chatFabricPairing *pair, int write );
void chatFabric_pair_init(chatFabricPairing *pair);
//enum chatFabricErrors chatFabric_processActions(chatFabricConfig *config, chatPacket *cp, chatPacket *reply);

enum chatFabricErrors 
chatFabric_controller(chatFabricConnection *c, chatFabricPairing *pair, chatFabricConfig *config,  chatFabricAction *a, msgbuffer *b);

enum chatFabricErrors 
chatFabric_device(chatFabricConnection *c, chatFabricPairing *pair, chatFabricConfig *config,  msgbuffer *b);

void
stateMachine (chatFabricConfig *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd, enum chatFabricErrors *e);


void
chatFabric_configParse(chatFabricConfig *config);

void
chatFabric_consetup( chatFabricConnection *c,  char *ip, int port );



#endif

