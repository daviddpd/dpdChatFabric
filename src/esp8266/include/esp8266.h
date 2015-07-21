#ifndef _esp8266_local_h_
#define _esp8266_local_h_

#include "driver/errno.h"


chatFabricConnection ESP_WORD_ALIGN c;
chatFabricConfig ESP_WORD_ALIGN config;  
msgbuffer ESP_WORD_ALIGN b;
uint32_t ESP_WORD_ALIGN status;
unsigned char ESP_WORD_ALIGN *tmp;
msgbuffer ESP_WORD_ALIGN mbuff;
uint8_t ESP_WORD_ALIGN wifiStatus;

chatPacket ESP_WORD_ALIGN cpArray[16];
int ESP_WORD_ALIGN cpStatus[16]; 
chatFabricPairing ESP_WORD_ALIGN pair[16]; 

#endif