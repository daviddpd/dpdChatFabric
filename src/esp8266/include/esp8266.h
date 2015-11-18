#ifndef _esp8266_local_h_
#define _esp8266_local_h_

#include "__attribute__.h"
#include "esp-cf-config.h"
#include "esp-cf-wifi.h"
#include "driver/errno.h"




uint32_t ESP_WORD_ALIGN status;
unsigned char ESP_WORD_ALIGN *tmp;
msgbuffer ESP_WORD_ALIGN mbuff;
uint8_t ESP_WORD_ALIGN wifiStatus;

chatPacket ESP_WORD_ALIGN *cp100;

chatPacket ESP_WORD_ALIGN cpArray[16];
int ESP_WORD_ALIGN cpStatus[16]; 
chatFabricPairing ESP_WORD_ALIGN pair[16]; 

int32_t heap, heapLast;

#define ESP_PARAM_START_SEC		0x3D
#define CP_ESP_PARAM_START_SEC 0x7a


#endif