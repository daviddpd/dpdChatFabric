#ifndef _espChatFabric_mdns_h_
#define _espChatFabric_mdns_h_
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
#include "uuuid2.h"

extern hostmeta_t hostMeta;
extern int mdns_status;


void espCfMdns();

#endif