//////////////////////////////////////////////////
// Simple NTP client for ESP8266.
// Copyright 2015 Richard A Burton
// richardaburton@gmail.com
// See license.txt for license terms.
//////////////////////////////////////////////////

#include <c_types.h>
#include <user_interface.h>
#include <espconn.h>
#include <osapi.h>
#include <mem.h>
#include <time.h>

#include "util.h"
#include "ntp.h"

// list of major public servers http://tf.nist.gov/tf-cgi/servers.cgi
/*
	NTP.ORG ntp pools

> host 0.pool.ntp.org
0.pool.ntp.org has address 208.75.88.4
0.pool.ntp.org has address 54.236.224.171
0.pool.ntp.org has address 108.61.73.244
0.pool.ntp.org has address 216.152.240.220

> host 1.pool.ntp.org
1.pool.ntp.org has address 129.6.15.29
1.pool.ntp.org has address 104.131.53.252
1.pool.ntp.org has address 96.244.96.19
1.pool.ntp.org has address 138.236.128.112

*/
#define NTP_SERVER_LENGTH 8

struct ip_info ntp_server[NTP_SERVER_LENGTH];

time_t ntp_unix_timestamp;
enum NTP_STATE ntp_status;
char ntp_status_str[128];
int ntp_select_server = -1;
struct ip_info ntpip;

/*
time_t ntp_unix_timestamp;
enum NTP_STATE ntp_status;
char ntp_status_str[128];
int ntp_select_server = -1;
uint32 ntpip = 0;
*/

extern int _GLOBAL_DEBUG;



static void CP_ICACHE_FLASH_ATTR ntp_udp_timeout(void *arg){
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "start" );

	os_timer_disarm(&ntp_timeout);
	//uart0_tx_buffer("ntp timout\r\n", 12);
	bzero(ntp_status_str, strlen(ntp_status_str));
//	strcpy(ntp_status_str,  "ntp timeout");
	os_sprintf(ntp_status_str, " [NTP] Timeout ; from server " IPSTR " ",  IP2STR(&ntpip.ip));

	ntp_status = NTP_STATE_TIMEOUT;

	// clean up connection
	if (pCon) {
		espconn_delete(pCon);
		os_free(pCon->proto.udp);
		os_free(pCon);
		pCon = 0;
	}
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "return" );
}

static void CP_ICACHE_FLASH_ATTR ntp_udp_recv(void *arg, char *pdata, unsigned short len) {
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "start" );

	struct tm *dt;
	time_t timestamp;
	ntp_t *ntp;

	os_timer_disarm(&ntp_timeout);

	// extract ntp time
	ntp = (ntp_t*)pdata;
	timestamp = ntp->trans_time[0] << 24 | ntp->trans_time[1] << 16 |ntp->trans_time[2] << 8 | ntp->trans_time[3];
	// convert to unix time
	timestamp -= 2208988800UL;
	// create tm struct
	dt = gmtime(&timestamp);

	
	// do something with it, like setting an rtc
	//ds1307_setTime(dt);
	// or just print it out
	ntp_unix_timestamp = timestamp;
	ntp_status = NTP_STATE_SET;

	bzero(ntp_status_str, strlen(ntp_status_str));

	os_sprintf(ntp_status_str, " [NTP] ===> %02d:%02d:%02d; %d ; from server " IPSTR " ", dt->tm_hour, dt->tm_min, dt->tm_sec, timestamp, IP2STR(&ntpip.ip) );
	

	// clean up connection
	if (pCon) {
		espconn_delete(pCon);
		os_free(pCon->proto.udp);
		os_free(pCon);
		pCon = 0;
	}
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "return" );
}

void CP_ICACHE_FLASH_ATTR ntp_get_time() {
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "starting" );

	ntp_t ntp;

	ntp_status = NTP_STATE_SETTING;

	IP4_ADDR(&ntp_server[0].ip, 208,75,88,4);
	IP4_ADDR(&ntp_server[1].ip, 54,236,224,171);
	IP4_ADDR(&ntp_server[2].ip, 108,61,73,244);
	IP4_ADDR(&ntp_server[3].ip, 216,152,240,220);
	IP4_ADDR(&ntp_server[4].ip, 129,6,15,29);
	IP4_ADDR(&ntp_server[5].ip, 104,131,53,252);
	IP4_ADDR(&ntp_server[6].ip, 96,244,96,19);
	IP4_ADDR(&ntp_server[7].ip, 138,236,128,112);

	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "ntp array set" );

	// set up the udp "connection"
	pCon = (struct espconn*)os_zalloc(sizeof(struct espconn));
	pCon->type = ESPCONN_UDP;
	pCon->state = ESPCONN_NONE;
	pCon->proto.udp = (esp_udp*)os_zalloc(sizeof(esp_udp));
	pCon->proto.udp->local_port = espconn_port();
	pCon->proto.udp->remote_port = 123;

	int mode = wifi_get_opmode();
	if (ntp_select_server == -1) {
		if ( mode == 1 || mode == 3 ) {
			struct ip_info ipconfig;	
			bzero(&ipconfig, sizeof(ipconfig) );
			wifi_get_ip_info(STATION_IF, &ipconfig);
			ntpip.ip.addr = ipconfig.gw.addr;
		} else {
			ntp_select_server == 0;
		}
	}

	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "selecting Ip address" );

	if ( ntp_select_server != -1 ) {
		ntpip.ip.addr = ntp_server[ntp_select_server].ip.addr;
	}	
	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "memcpy ip" );
	os_memcpy(&pCon->proto.udp->remote_ip, &ntpip.ip.addr, 4);
	ntp_select_server++;
	if (ntp_select_server >= NTP_SERVER_LENGTH ) {
		ntp_select_server = -1;
	}

	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "selected" );
	
	// create a really simple ntp request packet
	os_memset(&ntp, 0, sizeof(ntp_t));
	ntp.options = 0b00100011; // leap = 0, version = 4, mode = 3 (client)

	// set timeout timer
	os_timer_disarm(&ntp_timeout);
	os_timer_setfn(&ntp_timeout, (os_timer_func_t*)ntp_udp_timeout, pCon);
	os_timer_arm(&ntp_timeout, NTP_TIMEOUT_MS, 0);

	// send the ntp request
	espconn_create(pCon);
	espconn_regist_recvcb(pCon, ntp_udp_recv);
	espconn_sent(pCon, (uint8*)&ntp, sizeof(ntp_t));

	CHATFABRIC_DEBUG(_GLOBAL_DEBUG,  "return" );
}
