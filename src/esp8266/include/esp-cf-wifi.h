#ifndef _espChatFabric_wifi_h_
#define _espChatFabric_wifi_h_
#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "driver/gpio16.h"
#include "os_type.h"
#include "user_config.h"
#include "user_interface.h"
#include "driver/uart.h"
#include "espconn.h"
#include "mem.h"
#include "uuuid2.h"
#include <salsa20.h>
#include <poly1305-donna.h>
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "esp8266.h"
#include "util.h"
#include "sntp.h"

void (*callback)(void *config, chatPacket *cp,  chatFabricPairing *pair, chatPacket *reply, enum chatPacketCommands *replyCmd);	

void espWiFiInit();
void espCfWiFi_callBack(System_Event_t *evt);
void espCfWiFi_listen();



#endif