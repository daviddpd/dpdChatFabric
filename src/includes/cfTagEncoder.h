#ifndef cfTagEncoder_H
#define cfTagEncoder_H

#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "cfstructs.h"

void CP_ICACHE_FLASH_ATTR
cfTagEncoder( enum chatPacketTagData type, unsigned char *b, uint32_t *i, unsigned char tag,  uint32_t value, unsigned char*s, uint32_t len, uuuid2_t *uuid);

#define chatPacket_tagDataEncoder cfTagEncoder


#endif


