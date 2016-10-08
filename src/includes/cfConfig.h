#ifndef CFCONFIG_H
#define CFCONFIG_H

#include "uuuid2.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "cfstructs.h"

void CP_ICACHE_FLASH_ATTR cfConfigInit(chatFabricConfig *config);
void CP_ICACHE_FLASH_ATTR cfConfigRead(chatFabricConfig *config);
void CP_ICACHE_FLASH_ATTR cfConfigWrite(chatFabricConfig *config);

void CP_ICACHE_FLASH_ATTR
_cfConfigRead(chatFabricConfig *config, int fromStr, unsigned char* cstr, int cstr_len);

void CP_ICACHE_FLASH_ATTR
cfConfigSetFromStr(chatFabricConfig *config, unsigned char* cstr, int cstr_len);

void CP_ICACHE_FLASH_ATTR
cfConfigWrite(chatFabricConfig *config);

void CP_ICACHE_FLASH_ATTR 
_createKeyString (chatFabricConfig *config, msgbuffer *str);

void CP_ICACHE_FLASH_ATTR 
_createConfigString (chatFabricConfig *config, msgbuffer *str);

// void CP_ICACHE_FLASH_ATTR chatFabric_configParse(chatFabricConfig *config);

// #define chatFabric_configParse cfConfigParse;

#endif
