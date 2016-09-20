//////////////////////////////////////////////////
// Simple NTP client for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#ifndef __NTP_H__
#define __NTP_H__

#define NTP_TIMEOUT_MS 5000
#ifdef ESP8266
//#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#ifndef CP_ICACHE_FLASH_ATTR
#define CP_ICACHE_FLASH_ATTR __attribute__((section(".irom0.text")))
#endif
#define ESP_WORD_ALIGN __attribute__ ((aligned (4)))
#else
#define CP_ICACHE_FLASH_ATTR __attribute__(())
#define ESP_WORD_ALIGN __attribute__ (())
#endif



static os_timer_t ntp_timeout;
static struct espconn *pCon = 0;

enum NTP_STATE {	
	NTP_STATE_SET,
	NTP_STATE_SETTING,
	NTP_STATE_TIMEOUT,
};
typedef struct {
	uint8 options;
	uint8 stratum;
	uint8 poll;
	uint8 precision;
	uint32 root_delay;
	uint32 root_disp;
	uint32 ref_id;
	uint8 ref_time[8];
	uint8 orig_time[8];
	uint8 recv_time[8];
	uint8 trans_time[8];
} ntp_t;

void CP_ICACHE_FLASH_ATTR ntp_get_time();

#endif
