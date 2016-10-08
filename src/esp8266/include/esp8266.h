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

chatPacket ESP_WORD_ALIGN cpArray[16];
int ESP_WORD_ALIGN cpStatus[16]; 
chatFabricPairing ESP_WORD_ALIGN pair[16]; 

int32_t heap, heapLast;


// 0x3fc000

/*
 0fc000   1032192
 0fd000   1036288
 0fe000   1040384
 0ff000   1044480
*/

#define printf(...) os_printf( __VA_ARGS__ )

#define PRIV_PARAM_SAVE			0

#define PRIV_PARAM_START_SEC	0xFC

#define ESP_PARAM_START_SEC		0xFD
#define CP_ESP_PARAM_START_SEC 	0xFD


#endif