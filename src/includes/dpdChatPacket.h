
#ifndef DPDCHATPACKET_H
#define DPDCHATPACKET_H


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

#if defined(HAVE_SODIUM)
#include <sodium.h>
#endif 

#if  defined(HAVE_LOCAL_CRYPTO)
#include <salsa20.h>
#include <poly1305-donna.h>
#include <chacha20_simple.h>
#endif

#include "cfstructs.h"
#include "dpdChatFabric.h"


const char * stateLookup (enum chatPacketStates state);
const char * cmdLookup (enum chatPacketCommands cmd);

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

#endif
