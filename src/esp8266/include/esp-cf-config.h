#ifndef _espChatFabric_config_h_
#define _espChatFabric_config_h_
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
#include "uuid_wrapper.h"
#include "ntp.h"
#include <salsa20.h>
#include <poly1305-donna.h>
#include "dpdChatFabric.h"
#include "dpdChatPacket.h"
#include "esp8266.h"
#include "util.h"

enum deviceModes {	
	MODE_UNDEFINED,
	MODE_UNCONFIGURED,
	MODE_BOOTING,
	MODE_AP_UNPAIRED,
	MODE_AP_PAIRED,
	MODE_STA_NOWIFI,
	MODE_STA_UNPAIRED,
	MODE_STA_PAIRED,
	MODE_MENU_NONE,
	MODE_MENU_IN,
	MODE_MENU_APMODE,
	MODE_MENU_STAMODE,
	MODE_MENU_FACTORYRESET,
	MODE_MENU_UNPAIRALL,
	MODE_MENU_END,
} ESP_WORD_ALIGN;

extern enum deviceModes currentMode;
extern struct station_config stationConf;
extern unsigned char flashConfig[];
extern chatFabricConnection ESP_WORD_ALIGN c;
extern chatFabricConfig ESP_WORD_ALIGN config;  
extern msgbuffer ESP_WORD_ALIGN b;


void espCfConfigInit();


#endif