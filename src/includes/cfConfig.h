#ifndef CFCONFIG_H
#define CFCONFIG_H

#include <sys/types.h>
#include <sys/stat.h>
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "cfstructs.h"

void CP_ICACHE_FLASH_ATTR cfConfigInit(chatFabricConfig *config);
void CP_ICACHE_FLASH_ATTR cfConfigRead(chatFabricConfig *config);
void CP_ICACHE_FLASH_ATTR cfConfigWrite(chatFabricConfig *config);

// void CP_ICACHE_FLASH_ATTR chatFabric_configParse(chatFabricConfig *config);

// #define chatFabric_configParse cfConfigParse;

#endif
