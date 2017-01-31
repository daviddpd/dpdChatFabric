#ifndef _esp8266_local_h_
#define _esp8266_local_h_

#include "__attribute__.h"
#include "esp-cf-config.h"
#include "esp-cf-wifi.h"
#include "driver/errno.h"
#include <sys/types.h>



uint32_t ESP_WORD_ALIGN status;
unsigned char ESP_WORD_ALIGN *tmp;
msgbuffer ESP_WORD_ALIGN mbuff;
uint8_t ESP_WORD_ALIGN wifiStatus;

chatPacket ESP_WORD_ALIGN *cp100;

#define MAX_PAIRS 16

chatPacket ESP_WORD_ALIGN cpArray[MAX_PAIRS];
int ESP_WORD_ALIGN cpStatus[MAX_PAIRS]; 
chatFabricPairing ESP_WORD_ALIGN pair[MAX_PAIRS]; 

#define printf(...) os_printf( __VA_ARGS__ )

#define PRIV_PARAM_SAVE			0

#define PRIV_PARAM_START_SEC	0xFC

#define ESP_PARAM_START_SEC		0xFD
#define CP_ESP_PARAM_START_SEC 	0xFD


#endif